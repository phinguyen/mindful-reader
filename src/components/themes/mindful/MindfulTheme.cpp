#include "MindfulTheme.h"

#include <GfxRenderer.h>
#include <HalGPIO.h>
#include <I18n.h>

#include <algorithm>
#include <cstring>

#include "components/UITheme.h"
#include "components/icons/book.h"
#include "components/icons/bookmark.h"
#include "components/icons/check.h"
#include "components/icons/cover.h"
#include "components/icons/down24.h"
#include "components/icons/folder.h"
#include "components/icons/hotspot.h"
#include "components/icons/image.h"
#include "components/icons/left24.h"
#include "components/icons/right24.h"
#include "components/icons/saturn.h"
#include "components/icons/select24.h"
#include "components/icons/settings.h"
#include "components/icons/text.h"
#include "components/icons/transfer.h"
#include "components/icons/up24.h"
#include "components/icons/wifi.h"
#include "fontIds.h"

namespace {
constexpr int maxSubtitleWidth = 100;
constexpr int maxListValueWidth = 200;
constexpr int hPaddingInSelection = 8;
constexpr int topHintButtonY = 345;
constexpr int cornerRadius = 12;
constexpr int iconSize = 32;
constexpr int buttonMenuColumns = 3;
constexpr int buttonMenuRows = 2;
constexpr int buttonMenuTextGap = 6;

const uint8_t* iconForName(UIIcon icon) {
  switch (icon) {
    case UIIcon::Folder:
      return FolderIcon;
    case UIIcon::Book:
      return BookIcon;
    case UIIcon::Text:
      return TextIcon;
    case UIIcon::Image:
      return ImageIcon;
    case UIIcon::Recent:
      return BookmarkIcon;
    case UIIcon::Settings:
      return SettingsIcon;
    case UIIcon::Transfer:
      return TransferIcon;
    case UIIcon::Library:
      return SaturnIcon;
    case UIIcon::Wifi:
      return WifiIcon;
    case UIIcon::Hotspot:
      return HotspotIcon;
    default:
      return nullptr;
  }
}

const uint8_t* iconForButtonLabel(const char* label) {
  if (label == nullptr || label[0] == '\0') {
    return nullptr;
  }

  if (std::strcmp(label, tr(STR_DIR_UP)) == 0) {
    return Up24Icon;
  }
  if (std::strcmp(label, tr(STR_DIR_DOWN)) == 0) {
    return Down24Icon;
  }
  if (std::strcmp(label, tr(STR_DIR_LEFT)) == 0) {
    return Left24Icon;
  }
  if (std::strcmp(label, tr(STR_DIR_RIGHT)) == 0) {
    return Right24Icon;
  }
  if (std::strcmp(label, tr(STR_SELECT)) == 0 || std::strcmp(label, tr(STR_CONFIRM)) == 0 ||
      std::strcmp(label, tr(STR_OPEN)) == 0) {
    return Select24Icon;
  }
  if (std::strcmp(label, tr(STR_BACK)) == 0) {
    return Left24Icon;
  }
  if (std::strcmp(label, tr(STR_CONFIRM)) == 0 || std::strcmp(label, tr(STR_DONE)) == 0) {
    return CheckIcon;
  }

  return nullptr;
}

int computeIconVisualCenterYOffset(const uint8_t* bitmap, int size) {
  if (bitmap == nullptr || size <= 0) {
    return 0;
  }

  const int bytesPerRow = (size + 7) / 8;
  int firstInkRow = size;
  int lastInkRow = -1;

  // Icon bitmaps are 1-bit row-major. Treat non-white bits as visible ink and
  // center by visible bounds instead of raw canvas size.
  for (int y = 0; y < size; y++) {
    bool hasInk = false;
    for (int x = 0; x < size; x++) {
      const uint8_t rowByte = bitmap[y * bytesPerRow + x / 8];
      const bool isWhite = ((rowByte >> (7 - (x % 8))) & 0x01u) != 0;
      if (!isWhite) {
        hasInk = true;
        break;
      }
    }
    if (hasInk) {
      firstInkRow = std::min(firstInkRow, y);
      lastInkRow = y;
    }
  }

  if (lastInkRow < firstInkRow) {
    return 0;
  }

  const int boxCenterTimes2 = size - 1;
  const int inkCenterTimes2 = firstInkRow + lastInkRow;
  return (boxCenterTimes2 - inkCenterTimes2) / 2;
}
}  // namespace

void MindfulTheme::drawHeader(const GfxRenderer& renderer, Rect rect, const char* title, const char* subtitle) const {
  renderer.fillRect(rect.x, rect.y, rect.width, rect.height, false);

  const bool showBatteryPercentage =
      SETTINGS.hideBatteryPercentage != CrossPointSettings::HIDE_BATTERY_PERCENTAGE::HIDE_ALWAYS;
  // Position icon at right edge, drawBatteryRight will place text to the left
  const int batteryX = rect.x + rect.width - 12 - MindfulMetrics::values.batteryWidth;
  drawBatteryRight(
      renderer, Rect{batteryX, rect.y + 5, MindfulMetrics::values.batteryWidth, MindfulMetrics::values.batteryHeight},
      showBatteryPercentage);

  int maxTitleWidth =
      rect.width - MindfulMetrics::values.contentSidePadding * 2 - (subtitle != nullptr ? maxSubtitleWidth : 0);

  if (title) {
    auto truncatedTitle = renderer.truncatedText(UI_12_FONT_ID, title, maxTitleWidth, EpdFontFamily::BOLD);
    renderer.drawText(UI_12_FONT_ID, rect.x + MindfulMetrics::values.contentSidePadding,
                      rect.y + MindfulMetrics::values.batteryBarHeight + 3, truncatedTitle.c_str(), true,
                      EpdFontFamily::BOLD);
    renderer.drawLine(rect.x, rect.y + rect.height - 3, rect.x + rect.width - 1, rect.y + rect.height - 3, 3, true);
  }

  if (subtitle) {
    auto truncatedSubtitle = renderer.truncatedText(SMALL_FONT_ID, subtitle, maxSubtitleWidth, EpdFontFamily::REGULAR);
    int truncatedSubtitleWidth = renderer.getTextWidth(SMALL_FONT_ID, truncatedSubtitle.c_str());
    renderer.drawText(SMALL_FONT_ID,
                      rect.x + rect.width - MindfulMetrics::values.contentSidePadding - truncatedSubtitleWidth,
                      rect.y + 50, truncatedSubtitle.c_str(), true);
  }
}

void MindfulTheme::drawSubHeader(const GfxRenderer& renderer, Rect rect, const char* label,
                                 const char* rightLabel) const {
  int currentX = rect.x + MindfulMetrics::values.contentSidePadding;
  int rightSpace = MindfulMetrics::values.contentSidePadding;
  if (rightLabel) {
    auto truncatedRightLabel =
        renderer.truncatedText(SMALL_FONT_ID, rightLabel, maxListValueWidth, EpdFontFamily::REGULAR);
    int rightLabelWidth = renderer.getTextWidth(SMALL_FONT_ID, truncatedRightLabel.c_str());
    renderer.drawText(SMALL_FONT_ID, rect.x + rect.width - MindfulMetrics::values.contentSidePadding - rightLabelWidth,
                      rect.y + 7, truncatedRightLabel.c_str());
    rightSpace += rightLabelWidth + hPaddingInSelection;
  }

  auto truncatedLabel =
      renderer.truncatedText(UI_10_FONT_ID, label, rect.width - MindfulMetrics::values.contentSidePadding - rightSpace,
                             EpdFontFamily::REGULAR);
  renderer.drawText(UI_10_FONT_ID, currentX, rect.y + 6, truncatedLabel.c_str(), true, EpdFontFamily::REGULAR);

  renderer.drawLine(rect.x, rect.y + rect.height - 1, rect.x + rect.width - 1, rect.y + rect.height - 1, true);
}

void MindfulTheme::drawTabBar(const GfxRenderer& renderer, Rect rect, const std::vector<TabInfo>& tabs,
                              bool selected) const {
  int currentX = rect.x + MindfulMetrics::values.contentSidePadding;
  const int lineHeight = renderer.getLineHeight(UI_10_FONT_ID);
  const int textY = rect.y + (rect.height - lineHeight) / 2;

  for (const auto& tab : tabs) {
    const int textWidth = renderer.getTextWidth(UI_10_FONT_ID, tab.label, EpdFontFamily::REGULAR);
    const int tabX = currentX;
    const int tabW = textWidth + 2 * hPaddingInSelection;

    if (tab.selected) {
      if (selected) {
        renderer.fillRoundedRect(tabX, rect.y + 1, tabW, rect.height - 4, 6, Color::Black);
      } else {
        renderer.fillRectDither(tabX, rect.y, tabW, rect.height - 3, Color::LightGray);
        renderer.drawLine(tabX, rect.y + rect.height - 3, tabX + tabW, rect.y + rect.height - 3, 2, true);
      }
    }

    renderer.drawText(UI_10_FONT_ID, tabX + hPaddingInSelection, textY, tab.label, !(tab.selected && selected),
                      EpdFontFamily::REGULAR);

    currentX += textWidth + MindfulMetrics::values.tabSpacing + 2 * hPaddingInSelection;
  }

  renderer.drawLine(rect.x, rect.y + rect.height - 1, rect.x + rect.width - 1, rect.y + rect.height - 1, true);
}

void MindfulTheme::drawList(const GfxRenderer& renderer, Rect rect, int itemCount, int selectedIndex,
                            const std::function<std::string(int index)>& rowTitle,
                            const std::function<std::string(int index)>& rowSubtitle,
                            const std::function<UIIcon(int index)>& rowIcon,
                            const std::function<std::string(int index)>& rowValue, bool highlightValue) const {
  constexpr int contentPadding = 16;
  constexpr int subtitleGap = 2;
  constexpr int selectionBorderWidth = 2;
  constexpr int dividerThickness = 2;
  constexpr int dividerOnPixels = 1;
  constexpr int dividerOffPixels = 3;
  constexpr int dividerSelectedGap = 4;

  const int titleLineHeight = renderer.getLineHeight(UI_12_FONT_ID);
  const int subtitleLineHeight = renderer.getLineHeight(SMALL_FONT_ID);
  const bool hasSubtitleColumn = (rowSubtitle != nullptr);
  const int minRowHeight = hasSubtitleColumn ? (contentPadding * 2 + titleLineHeight + subtitleGap + subtitleLineHeight)
                                             : (contentPadding * 2 + titleLineHeight);

  int rowHeight =
      hasSubtitleColumn ? MindfulMetrics::values.listWithSubtitleRowHeight : MindfulMetrics::values.listRowHeight;
  rowHeight = std::max(rowHeight, minRowHeight);
  int pageItems = rect.height / rowHeight;
  if (pageItems <= 0) {
    return;
  }

  const int totalPages = (itemCount + pageItems - 1) / pageItems;
  if (totalPages > 1) {
    const int scrollAreaHeight = rect.height;

    // Draw scroll bar
    const int scrollBarHeight = (scrollAreaHeight * pageItems) / itemCount;
    const int currentPage = selectedIndex / pageItems;
    const int scrollBarY = rect.y + ((scrollAreaHeight - scrollBarHeight) * currentPage) / (totalPages - 1);
    const int scrollBarX = rect.x + rect.width - MindfulMetrics::values.scrollBarRightOffset;
    renderer.drawLine(scrollBarX, rect.y, scrollBarX, rect.y + scrollAreaHeight, true);
    renderer.fillRect(scrollBarX - MindfulMetrics::values.scrollBarWidth, scrollBarY,
                      MindfulMetrics::values.scrollBarWidth, scrollBarHeight, true);
  }

  int contentWidth =
      rect.width -
      (totalPages > 1 ? (MindfulMetrics::values.scrollBarWidth + MindfulMetrics::values.scrollBarRightOffset) : 1);

  // Draw all items
  const auto pageStartIndex = selectedIndex / pageItems * pageItems;
  for (int i = pageStartIndex; i < itemCount && i < pageStartIndex + pageItems; i++) {
    const int itemY = rect.y + (i % pageItems) * rowHeight;
    const int rowX = rect.x + MindfulMetrics::values.contentSidePadding;
    const int rowWidth = contentWidth - MindfulMetrics::values.contentSidePadding * 2;
    const bool isSelected = (i == selectedIndex);

    if (isSelected) {
      renderer.drawRoundedRect(rowX, itemY, rowWidth, rowHeight, selectionBorderWidth, cornerRadius, true);
    }

    std::string subtitleText = "";
    if (rowSubtitle != nullptr) {
      subtitleText = rowSubtitle(i);
    }
    const bool hasSubtitleText = hasSubtitleColumn && !subtitleText.empty();

    int contentX = rowX + contentPadding;
    int rowTextWidth = rowWidth - contentPadding * 2;

    // Draw name
    int valueWidth = 0;
    int valueTextWidth = 0;
    std::string valueText = "";
    if (rowValue != nullptr) {
      valueText = rowValue(i);
      valueText = renderer.truncatedText(UI_10_FONT_ID, valueText.c_str(), maxListValueWidth);
      valueTextWidth = renderer.getTextWidth(UI_10_FONT_ID, valueText.c_str());
      valueWidth = valueTextWidth + hPaddingInSelection;
      rowTextWidth -= valueWidth;
    }

    if (rowIcon != nullptr) {
      rowTextWidth -= iconSize + hPaddingInSelection;
      contentX += iconSize + hPaddingInSelection;
    }

    const int textBlockHeight = titleLineHeight + (hasSubtitleText ? (subtitleGap + subtitleLineHeight) : 0);
    const int textTopY = itemY + std::max(0, (rowHeight - textBlockHeight) / 2);
    const int titleY = textTopY;

    auto itemName = rowTitle(i);
    auto item = renderer.truncatedText(UI_12_FONT_ID, itemName.c_str(), rowTextWidth);
    renderer.drawText(UI_12_FONT_ID, contentX, titleY, item.c_str(), true);

    if (rowIcon != nullptr) {
      UIIcon icon = rowIcon(i);
      const uint8_t* iconBitmap = iconForName(icon);
      if (iconBitmap != nullptr) {
        int iconY = itemY + std::max(0, (rowHeight - iconSize) / 2);
        if (!hasSubtitleText) {
          iconY += computeIconVisualCenterYOffset(iconBitmap, iconSize);
        }
        renderer.drawIcon(iconBitmap, rowX + contentPadding, iconY, iconSize, iconSize);
      }
    }

    if (hasSubtitleText) {
      auto subtitle = renderer.truncatedText(SMALL_FONT_ID, subtitleText.c_str(), rowTextWidth);
      renderer.drawText(SMALL_FONT_ID, contentX, titleY + titleLineHeight + subtitleGap, subtitle.c_str(), true);
    }

    // Draw value
    if (!valueText.empty()) {
      const int valueLineHeight = renderer.getLineHeight(UI_10_FONT_ID);
      const int valueX = rowX + rowWidth - contentPadding - valueWidth;
      const int valueY = itemY + std::max(0, (rowHeight - valueLineHeight) / 2);
      renderer.drawText(UI_10_FONT_ID, valueX, valueY, valueText.c_str(), true);

      if (isSelected && highlightValue) {
        constexpr int selectedValueUnderlineThickness = 3;
        const int underlineY =
            std::min(itemY + rowHeight - selectedValueUnderlineThickness, valueY + valueLineHeight + 1);
        const int underlineEndX = valueX + std::max(0, valueTextWidth - 1);
        renderer.drawLine(valueX, underlineY, underlineEndX, underlineY, selectedValueUnderlineThickness, true);
      }
    }

    const bool hasNextVisibleItem = (i + 1 < itemCount) && (i + 1 < pageStartIndex + pageItems);
    if (hasNextVisibleItem) {
      int dividerYStart = itemY + rowHeight - dividerThickness;
      if (isSelected) {
        // Keep divider visible below selected row with a small visual gap.
        dividerYStart += dividerSelectedGap;
      } else if (i + 1 == selectedIndex) {
        // Keep divider visible above selected row with a small visual gap.
        dividerYStart -= dividerSelectedGap;
      }
      const int dividerStartX = rowX;
      const int dividerEndX = rowX + rowWidth;
      renderer.drawPatternHLine(dividerStartX, dividerEndX, dividerYStart, dividerThickness, dividerOnPixels,
                                dividerOffPixels, true);
    }
  }
}

void MindfulTheme::drawButtonHints(GfxRenderer& renderer, const char* btn1, const char* btn2, const char* btn3,
                                   const char* btn4) const {
  const GfxRenderer::Orientation orig_orientation = renderer.getOrientation();
  renderer.setOrientation(GfxRenderer::Orientation::Portrait);

  const int pageHeight = renderer.getScreenHeight();
  constexpr int buttonWidth = 106;
  constexpr int buttonIconSize = 24;
  constexpr int smallButtonHeight = 15;
  constexpr int buttonHeight = MindfulMetrics::values.buttonHintsHeight;
  constexpr int buttonY = MindfulMetrics::values.buttonHintsHeight;  // Distance from bottom
  // Keep a fixed 4 px gap between button bounds.
  constexpr int x4ButtonPositions[] = {22, 132, 242, 352};
  constexpr int x3ButtonPositions[] = {46, 156, 266, 376};
  const int* buttonPositions = gpio.deviceIsX3() ? x3ButtonPositions : x4ButtonPositions;
  const char* labels[] = {btn1, btn2, btn3, btn4};
  const int fullButtonTop = pageHeight - buttonY;
  const int smallButtonTop = pageHeight - smallButtonHeight;

  for (int i = 0; i < 4; i++) {
    const int x = buttonPositions[i];
    const char* label = labels[i];
    if (label != nullptr && label[0] != '\0') {
      renderer.fillRoundedRect(x, fullButtonTop, buttonWidth, buttonHeight, cornerRadius, Color::White);
      renderer.drawRoundedRect(x, fullButtonTop, buttonWidth, buttonHeight, 1, cornerRadius, true, true, false, false,
                               true);
      if (const uint8_t* icon = iconForButtonLabel(label)) {
        const int iconX = x + (buttonWidth - buttonIconSize) / 2;
        const int iconY = fullButtonTop + (buttonHeight - buttonIconSize) / 2;
        renderer.drawIcon(icon, iconX, iconY, buttonIconSize, buttonIconSize);
      } else {
        const int textWidth = renderer.getTextWidth(SMALL_FONT_ID, label);
        const int textX = x + (buttonWidth - textWidth) / 2;
        const int textY = fullButtonTop + (buttonHeight - renderer.getLineHeight(SMALL_FONT_ID)) / 2;
        renderer.drawText(SMALL_FONT_ID, textX, textY, label);
      }
    } else {
      renderer.fillRoundedRect(x, smallButtonTop, buttonWidth, smallButtonHeight, cornerRadius, Color::White);
      renderer.drawRoundedRect(x, smallButtonTop, buttonWidth, smallButtonHeight, 1, cornerRadius, true, true, false,
                               false, true);
    }
  }

  renderer.setOrientation(orig_orientation);
}

void MindfulTheme::drawSideButtonHints(const GfxRenderer& renderer, const char* topBtn, const char* bottomBtn) const {
  const int screenWidth = renderer.getScreenWidth();
  constexpr int buttonWidth = MindfulMetrics::values.sideButtonHintsWidth;  // Width on screen (height when rotated)
  constexpr int buttonHeight = 78;                                          // Height on screen (width when rotated)
  constexpr int buttonMargin = 0;
  constexpr int iconSize = 24;

  if (gpio.deviceIsX3()) {
    // X3 layout: Up on left side, Down on right side, positioned higher
    constexpr int x3ButtonY = 155;

    if (topBtn != nullptr && topBtn[0] != '\0') {
      renderer.drawRoundedRect(buttonMargin, x3ButtonY, buttonWidth, buttonHeight, 1, cornerRadius, false, true, false,
                               true, true);
      const uint8_t* icon = (std::strcmp(topBtn, ">") == 0) ? Right24Icon : Left24Icon;
      const int iconX = buttonMargin + (buttonWidth - iconSize) / 2;
      const int iconY = x3ButtonY + (buttonHeight - iconSize) / 2;
      renderer.drawIcon(icon, iconX, iconY, iconSize, iconSize);
    }

    if (bottomBtn != nullptr && bottomBtn[0] != '\0') {
      const int rightX = screenWidth - buttonWidth;
      renderer.drawRoundedRect(rightX, x3ButtonY, buttonWidth, buttonHeight, 1, cornerRadius, true, false, true, false,
                               true);
      const uint8_t* icon = (std::strcmp(bottomBtn, ">") == 0) ? Up24Icon : Down24Icon;
      const int iconX = rightX + (buttonWidth - iconSize) / 2;
      const int iconY = x3ButtonY + (buttonHeight - iconSize) / 2;
      renderer.drawIcon(icon, iconX, iconY, iconSize, iconSize);
    }
  } else {
    // X4 layout: Both buttons stacked on right side
    const char* labels[] = {topBtn, bottomBtn};
    const int x = screenWidth - buttonWidth;

    if (topBtn != nullptr && topBtn[0] != '\0') {
      renderer.drawRoundedRect(x, topHintButtonY, buttonWidth, buttonHeight, 1, cornerRadius, true, false, true, false,
                               true);
    }

    if (bottomBtn != nullptr && bottomBtn[0] != '\0') {
      renderer.drawRoundedRect(x, topHintButtonY + buttonHeight + 5, buttonWidth, buttonHeight, 1, cornerRadius, true,
                               false, true, false, true);
    }

    for (int i = 0; i < 2; i++) {
      if (labels[i] != nullptr && labels[i][0] != '\0') {
        const int y = topHintButtonY + (i * buttonHeight) + 5;
        const uint8_t* icon = (std::strcmp(labels[i], ">") == 0) ? Up24Icon : Down24Icon;
        const int iconX = x + (buttonWidth - iconSize) / 2;
        const int iconY = y + (buttonHeight - iconSize) / 2;
        renderer.drawIcon(icon, iconX, iconY, iconSize, iconSize);
      }
    }
  }
}

void MindfulTheme::drawKeyboardKey(const GfxRenderer& renderer, Rect rect, const char* label,
                                   const bool isSelected) const {
  if (isSelected) {
    renderer.fillRoundedRect(rect.x, rect.y, rect.width, rect.height, cornerRadius, Color::Black);
  }

  const int textWidth = renderer.getTextWidth(UI_12_FONT_ID, label);
  const int textX = rect.x + (rect.width - textWidth) / 2;
  const int textY = rect.y + (rect.height - renderer.getLineHeight(UI_12_FONT_ID)) / 2;
  renderer.drawText(UI_12_FONT_ID, textX, textY, label, !isSelected);
}
void MindfulTheme::drawButtonMenu(GfxRenderer& renderer, Rect rect, int buttonCount, int selectedIndex,
                                  const std::function<std::string(int index)>& buttonLabel,
                                  const std::function<UIIcon(int index)>& rowIcon) const {
  constexpr int maxGridItems = buttonMenuColumns * buttonMenuRows;
  const int itemsToDraw = std::min(buttonCount, maxGridItems);
  if (itemsToDraw <= 0) {
    return;
  }

  const int cellGap = MindfulMetrics::values.menuSpacing;
  const int contentX = rect.x + MindfulMetrics::values.contentSidePadding;
  const int contentW = rect.width - MindfulMetrics::values.contentSidePadding * 2;

  // Anchor menu from bottom-up, and keep it above the button hints area.
  const GfxRenderer::Orientation origOrientation = renderer.getOrientation();
  renderer.setOrientation(GfxRenderer::Orientation::Portrait);
  const int screenHeightPortrait = renderer.getScreenHeight();
  renderer.setOrientation(origOrientation);

  const int buttonHintsTopY = screenHeightPortrait - MindfulMetrics::values.buttonHintsHeight - cellGap;
  const int menuBottomY = std::min(rect.y + rect.height, buttonHintsTopY);
  const int availableMenuHeight = menuBottomY - rect.y;
  if (availableMenuHeight <= 0) {
    return;
  }

  const int lineHeight = renderer.getLineHeight(UI_12_FONT_ID);
  const int iconAndTextHeight = iconSize + buttonMenuTextGap + lineHeight;
  const int cellW = (contentW - (buttonMenuColumns - 1) * cellGap) / buttonMenuColumns;
  const int cellH = iconAndTextHeight + 24;
  if (cellW <= 0 || cellH <= 0) {
    return;
  }

  const int gridW = cellW * buttonMenuColumns + (buttonMenuColumns - 1) * cellGap;
  const int gridH = cellH * buttonMenuRows + (buttonMenuRows - 1) * cellGap;
  if (gridH > availableMenuHeight) {
    return;
  }
  const int gridX = contentX + (contentW - gridW) / 2;
  const int gridY = menuBottomY - gridH;

  for (int i = 0; i < itemsToDraw; ++i) {
    const int col = i % buttonMenuColumns;
    const int row = i / buttonMenuColumns;
    Rect tileRect = Rect{gridX + col * (cellW + cellGap), gridY + row * (cellH + cellGap), cellW, cellH};

    const bool selected = selectedIndex == i;
    if (selected) {
      renderer.fillRoundedRect(tileRect.x, tileRect.y, tileRect.width, tileRect.height, cornerRadius, Color::LightGray);
    }
    renderer.drawRoundedRect(tileRect.x, tileRect.y, tileRect.width, tileRect.height, 1, cornerRadius, true);

    std::string labelStr = buttonLabel(i);
    std::string truncatedLabel = renderer.truncatedText(UI_10_FONT_ID, labelStr.c_str(), tileRect.width - 8);
    const int textWidth = renderer.getTextWidth(UI_10_FONT_ID, truncatedLabel.c_str());

    const int blockTop = std::max(tileRect.y, tileRect.y + (tileRect.height - iconAndTextHeight) / 2);
    const int iconX = tileRect.x + (tileRect.width - iconSize) / 2;
    const int textX = tileRect.x + (tileRect.width - textWidth) / 2;
    const int textY = blockTop + iconSize + buttonMenuTextGap;

    if (rowIcon != nullptr) {
      const UIIcon icon = rowIcon(i);
      const uint8_t* iconBitmap = iconForName(icon);
      if (iconBitmap != nullptr) {
        const int iconY = blockTop + computeIconVisualCenterYOffset(iconBitmap, iconSize);
        renderer.drawIcon(iconBitmap, iconX, iconY, iconSize, iconSize);
      }
    }

    renderer.drawText(UI_10_FONT_ID, textX, textY, truncatedLabel.c_str(), true);
  }
}
