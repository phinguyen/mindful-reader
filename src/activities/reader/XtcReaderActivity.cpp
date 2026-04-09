/**
 * XtcReaderActivity.cpp
 *
 * XTC ebook reader activity implementation
 * Displays pre-rendered XTC pages on e-ink display
 */

#include "XtcReaderActivity.h"

#include <FsHelpers.h>
#include <GfxRenderer.h>
#include <HalStorage.h>
#include <I18n.h>

#include "CrossPointSettings.h"
#include "CrossPointState.h"
#include "MappedInputManager.h"
#include "RecentBooksStore.h"
#include "XtcReaderChapterSelectionActivity.h"
#include "components/UITheme.h"
#include "fontIds.h"

namespace {
constexpr unsigned long skipPageMs = 700;
constexpr unsigned long goHomeMs = 1000;
// Strip width for 2-bit column-major rendering to reduce peak RAM usage.
// Each strip uses STRIP_COLS * colBytes * 2 planes bytes.
// For 800-height display: colBytes=100, STRIP_COLS=48 → 48*100*2 = 9,600 bytes
// per strip.
constexpr uint16_t STRIP_COLS = 48;
} // namespace

void XtcReaderActivity::onEnter() {
  Activity::onEnter();

  if (!xtc) {
    return;
  }

  xtc->setupCacheDir();

  // Load saved progress
  loadProgress();

  // Save current XTC as last opened book and add to recent books
  APP_STATE.openEpubPath = xtc->getPath();
  APP_STATE.saveToFile();
  RECENT_BOOKS.addBook(xtc->getPath(), xtc->getTitle(), xtc->getAuthor(),
                       xtc->getThumbBmpPath());

  // Trigger first update
  requestUpdate();
}

void XtcReaderActivity::onExit() {
  Activity::onExit();

  // Request half refresh for the next screen to clear accumulated reader
  // ghosting
  renderer.requestNextHalfRefresh();

  APP_STATE.readerActivityLoadCount = 0;
  APP_STATE.saveToFile();
  xtc.reset();
}

void XtcReaderActivity::loop() {
  // Enter chapter selection activity
  if (mappedInput.wasReleased(MappedInputManager::Button::Confirm)) {
    if (xtc && xtc->hasChapters() && !xtc->getChapters().empty()) {
      startActivityForResult(
          std::make_unique<XtcReaderChapterSelectionActivity>(
              renderer, mappedInput, xtc, currentPage),
          [this](const ActivityResult &result) {
            if (!result.isCancelled) {
              currentPage = std::get<PageResult>(result.data).page;
            }
          });
    }
  }

  // Long press BACK (1s+) goes to file selection
  if (mappedInput.isPressed(MappedInputManager::Button::Back) &&
      mappedInput.getHeldTime() >= goHomeMs) {
    activityManager.goToFileBrowser(xtc ? xtc->getPath() : "");
    return;
  }

  // Short press BACK goes directly to home
  if (mappedInput.wasReleased(MappedInputManager::Button::Back) &&
      mappedInput.getHeldTime() < goHomeMs) {
    onGoHome();
    return;
  }

  // When long-press chapter skip is disabled, turn pages on press instead of
  // release.
  const bool usePressForPageTurn = !SETTINGS.longPressChapterSkip;
  const bool prevTriggered =
      usePressForPageTurn
          ? (mappedInput.wasPressed(MappedInputManager::Button::PageBack) ||
             mappedInput.wasPressed(MappedInputManager::Button::Left))
          : (mappedInput.wasReleased(MappedInputManager::Button::PageBack) ||
             mappedInput.wasReleased(MappedInputManager::Button::Left));
  const bool powerPageTurn =
      SETTINGS.shortPwrBtn == CrossPointSettings::SHORT_PWRBTN::PAGE_TURN &&
      mappedInput.wasReleased(MappedInputManager::Button::Power);
  const bool nextTriggered =
      usePressForPageTurn
          ? (mappedInput.wasPressed(MappedInputManager::Button::PageForward) ||
             powerPageTurn ||
             mappedInput.wasPressed(MappedInputManager::Button::Right))
          : (mappedInput.wasReleased(MappedInputManager::Button::PageForward) ||
             powerPageTurn ||
             mappedInput.wasReleased(MappedInputManager::Button::Right));

  if (!prevTriggered && !nextTriggered) {
    return;
  }

  // Handle end of book
  if (currentPage >= xtc->getPageCount()) {
    currentPage = xtc->getPageCount() - 1;
    requestUpdate();
    return;
  }

  const bool skipPages =
      SETTINGS.longPressChapterSkip && mappedInput.getHeldTime() > skipPageMs;
  const int skipAmount = skipPages ? 10 : 1;

  if (prevTriggered) {
    if (currentPage >= static_cast<uint32_t>(skipAmount)) {
      currentPage -= skipAmount;
    } else {
      currentPage = 0;
    }
    requestUpdate();
  } else if (nextTriggered) {
    currentPage += skipAmount;
    if (currentPage >= xtc->getPageCount()) {
      currentPage = xtc->getPageCount(); // Allow showing "End of book"
    }
    requestUpdate();
  }
}

void XtcReaderActivity::render(RenderLock &&) {
  if (!xtc) {
    return;
  }

  // Bounds check
  if (currentPage >= xtc->getPageCount()) {
    // Show end of book screen
    renderer.clearScreen();
    renderer.drawCenteredText(UI_12_FONT_ID, 300, tr(STR_END_OF_BOOK), true,
                              EpdFontFamily::BOLD);
    renderer.displayBuffer();
    return;
  }

  renderPage();
  saveProgress();
}

// Render 2-bit grayscale page from the full page buffer. The parser exposes
// full-page reads, so we decode the two column-major planes in place.
static bool renderPage2bit(GfxRenderer& renderer, Xtc* xtc, uint32_t currentPage, int& pagesUntilFullRefresh) {
  const uint16_t pageWidth = xtc->getPageWidth();
  const uint16_t pageHeight = xtc->getPageHeight();
  const size_t colBytes = (pageHeight + 7) / 8;
  const size_t planeBytes = static_cast<size_t>(pageWidth) * colBytes;
  const size_t pageBufferSize = planeBytes * 2;

  LOG_DBG("XTR", "2-bit render: page=%lu, pageBuf=%lu bytes, freeHeap=%lu", currentPage,
          static_cast<unsigned long>(pageBufferSize), (unsigned long)ESP.getFreeHeap());

  uint8_t* pageBuffer = static_cast<uint8_t*>(malloc(pageBufferSize));
  if (!pageBuffer) {
    LOG_ERR("XTR", "Failed to allocate page buffer (%lu bytes)",
            static_cast<unsigned long>(pageBufferSize));
    return false;
  }

  const uint8_t* plane1 = pageBuffer;
  const uint8_t* plane2 = pageBuffer + planeBytes;

  // Rendering pass helper: draw pixels based on pass type.
  bool passOk = true;
  auto doPass = [&](uint8_t passFlag, bool drawBlack) {
    for (uint16_t x = 0; x < pageWidth && passOk; x++) {
      const size_t physCol = pageWidth - 1 - x;
      const size_t colBase = physCol * colBytes;
      for (uint16_t y = 0; y < pageHeight; y++) {
        const size_t byteInCol = y / 8;
        const size_t bitInByte = 7 - (y % 8);
        const size_t byteOffset = colBase + byteInCol;
        const uint8_t bit1 = (plane1[byteOffset] >> bitInByte) & 1;
        const uint8_t bit2 = (plane2[byteOffset] >> bitInByte) & 1;
        const uint8_t pv = (bit1 << 1) | bit2;

        bool draw = false;
        switch (passFlag) {
          case 0:
            draw = (pv >= 1);
            break;
          case 1:
            draw = (pv == 1);
            break;
          case 2:
            draw = (pv == 1 || pv == 2);
            break;
          case 3:
            draw = (pv >= 1);
            break;
        }
        if (draw) {
          renderer.drawPixel(x, y, drawBlack);
        }
      }
    }
  };

  const size_t bytesRead = xtc->loadPage(currentPage, pageBuffer, pageBufferSize);
  if (bytesRead != pageBufferSize) {
    LOG_ERR("XTR", "Failed to load page %lu (%lu of %lu bytes)", currentPage,
            static_cast<unsigned long>(bytesRead),
            static_cast<unsigned long>(pageBufferSize));
    free(pageBuffer);
    return false;
  }

  renderer.clearScreen();

  // Pass 1: BW buffer - draw all non-white pixels as black
  doPass(0, true);
  if (!passOk) {
    free(pageBuffer);
    return false;
  }

  // Display BW
  if (pagesUntilFullRefresh <= 1) {
    renderer.displayBuffer(HalDisplay::HALF_REFRESH);
    pagesUntilFullRefresh = SETTINGS.getRefreshFrequency();
  } else {
    renderer.displayBuffer();
    pagesUntilFullRefresh--;
  }

  // Pass 2: LSB buffer - dark grey only
  renderer.clearScreen(0x00);
  doPass(1, false);
  if (!passOk) {
    free(pageBuffer);
    return false;
  }
  renderer.copyGrayscaleLsbBuffers();

  // Pass 3: MSB buffer - both grays
  renderer.clearScreen(0x00);
  doPass(2, false);
  if (!passOk) {
    free(pageBuffer);
    return false;
  }
  renderer.copyGrayscaleMsbBuffers();

  // Display grayscale overlay
  renderer.displayGrayBuffer();

  // Pass 4: Re-render BW to framebuffer (restore for next frame)
  renderer.clearScreen();
  doPass(3, true);
  if (!passOk) {
    free(pageBuffer);
    return false;
  }
  renderer.cleanupGrayscaleWithFrameBuffer();

  free(pageBuffer);
  LOG_DBG("XTR", "Rendered page %lu (2-bit, freeHeap=%lu)",
          currentPage + 1, (unsigned long)ESP.getFreeHeap());
  return true;
}

void XtcReaderActivity::renderPage() {
  const uint16_t pageWidth = xtc->getPageWidth();
  const uint16_t pageHeight = xtc->getPageHeight();
  const uint8_t bitDepth = xtc->getBitDepth();

  // Calculate buffer size for one page
  // XTG (1-bit): Row-major, ((width+7)/8) * height bytes
  // XTH (2-bit): Two bit planes, column-major, ((width * height + 7) / 8) * 2
  // bytes

  if (bitDepth == 2) {
    LOG_DBG("XTR", "Free heap before 2-bit render: %lu",
            (unsigned long)ESP.getFreeHeap());
    if (!renderPage2bit(renderer, xtc.get(), currentPage, pagesUntilFullRefresh)) {
      renderer.clearScreen();
      renderer.drawCenteredText(UI_12_FONT_ID, 300, tr(STR_MEMORY_ERROR), true,
                                EpdFontFamily::BOLD);
      renderer.displayBuffer();
    }
    return;
  }

  // 1-bit mode: ((width+7)/8) * height bytes = 48KB for 480x800
  size_t pageBufferSize = ((pageWidth + 7) / 8) * pageHeight;
  LOG_DBG("XTR", "Free heap before 1-bit alloc: %lu, need: %lu",
          (unsigned long)ESP.getFreeHeap(), (unsigned long)pageBufferSize);

  // Allocate page buffer
  uint8_t *pageBuffer = static_cast<uint8_t *>(malloc(pageBufferSize));
  if (!pageBuffer) {
    LOG_ERR("XTR", "Failed to allocate page buffer (%lu bytes)",
            pageBufferSize);
    renderer.clearScreen();
    renderer.drawCenteredText(UI_12_FONT_ID, 300, tr(STR_MEMORY_ERROR), true,
                              EpdFontFamily::BOLD);
    renderer.displayBuffer();
    return;
  }

  // Load page data
  size_t bytesRead = xtc->loadPage(currentPage, pageBuffer, pageBufferSize);
  if (bytesRead == 0) {
    LOG_ERR("XTR", "Failed to load page %lu", currentPage);
    free(pageBuffer);
    renderer.clearScreen();
    renderer.drawCenteredText(UI_12_FONT_ID, 300, tr(STR_PAGE_LOAD_ERROR), true,
                              EpdFontFamily::BOLD);
    renderer.displayBuffer();
    return;
  }

  // Clear screen first
  renderer.clearScreen();

  // 1-bit mode: 8 pixels per byte, MSB first
  // XTC/XTCH pages are pre-rendered with status bar included, so render full page
  const size_t srcRowBytes = (pageWidth + 7) / 8; // 60 bytes for 480 width

  for (uint16_t srcY = 0; srcY < pageHeight; srcY++) {
    const size_t srcRowStart = srcY * srcRowBytes;

    for (uint16_t srcX = 0; srcX < pageWidth; srcX++) {
      // Read source pixel (MSB first, bit 7 = leftmost pixel)
      const size_t srcByte = srcRowStart + srcX / 8;
      const size_t srcBit = 7 - (srcX % 8);
      const bool isBlack =
          !((pageBuffer[srcByte] >> srcBit) & 1); // XTC: 0 = black, 1 = white

      if (isBlack) {
        renderer.drawPixel(srcX, srcY, true);
      }
    }
  }
  // White pixels are already cleared by clearScreen()

  free(pageBuffer);

  // XTC pages already have status bar pre-rendered, no need to add our own

  // Display with appropriate refresh
  if (pagesUntilFullRefresh <= 1) {
    renderer.displayBuffer(HalDisplay::HALF_REFRESH);
    pagesUntilFullRefresh = SETTINGS.getRefreshFrequency();
  } else {
    renderer.displayBuffer();
    pagesUntilFullRefresh--;
  }

  LOG_DBG("XTR", "Rendered page %lu/%lu (%u-bit)", currentPage + 1,
          xtc->getPageCount(), bitDepth);
}

void XtcReaderActivity::saveProgress() const {
  FsFile f;
  if (Storage.openFileForWrite("XTR", xtc->getCachePath() + "/progress.bin",
                               f)) {
    uint8_t data[4];
    data[0] = currentPage & 0xFF;
    data[1] = (currentPage >> 8) & 0xFF;
    data[2] = (currentPage >> 16) & 0xFF;
    data[3] = (currentPage >> 24) & 0xFF;
    f.write(data, 4);
    f.close();
  }
}

void XtcReaderActivity::loadProgress() {
  FsFile f;
  if (Storage.openFileForRead("XTR", xtc->getCachePath() + "/progress.bin",
                              f)) {
    uint8_t data[4];
    if (f.read(data, 4) == 4) {
      currentPage =
          data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
      LOG_DBG("XTR", "Loaded progress: page %lu", currentPage);

      // Validate page number
      if (currentPage >= xtc->getPageCount()) {
        currentPage = 0;
      }
    }
    f.close();
  }
}
