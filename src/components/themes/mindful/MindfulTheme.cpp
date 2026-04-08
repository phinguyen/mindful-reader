#include "MindfulTheme.h"

#include <GfxRenderer.h>

#include <algorithm>

#include "components/UITheme.h"
#include "components/icons/book.h"
#include "components/icons/book24.h"
#include "components/icons/cover.h"
#include "components/icons/file24.h"
#include "components/icons/folder.h"
#include "components/icons/folder24.h"
#include "components/icons/hotspot.h"
#include "components/icons/image24.h"
#include "components/icons/library.h"
#include "components/icons/recent.h"
#include "components/icons/settings2.h"
#include "components/icons/text24.h"
#include "components/icons/transfer.h"
#include "components/icons/wifi.h"
#include "fontIds.h"

namespace {
constexpr int maxSubtitleWidth = 100;
constexpr int maxListValueWidth = 200;
constexpr int hPaddingInSelection = 8;
constexpr int cornerRadius = 12;
constexpr int mainMenuIconSize = 32;
constexpr int listIconSize = 24;

const uint8_t* iconForName(UIIcon icon, int size) {
  if (size == 24) {
    switch (icon) {
      case UIIcon::Folder:
        return Folder24Icon;
      case UIIcon::Text:
        return Text24Icon;
      case UIIcon::Image:
        return Image24Icon;
      case UIIcon::Book:
        return Book24Icon;
      case UIIcon::File:
        return File24Icon;
      default:
        return nullptr;
    }
  } else if (size == 32) {
    switch (icon) {
      case UIIcon::Folder:
        return FolderIcon;
      case UIIcon::Book:
        return BookIcon;
      case UIIcon::Recent:
        return RecentIcon;
      case UIIcon::Settings:
        return Settings2Icon;
      case UIIcon::Transfer:
        return TransferIcon;
      case UIIcon::Library:
        return LibraryIcon;
      case UIIcon::Wifi:
        return WifiIcon;
      case UIIcon::Hotspot:
        return HotspotIcon;
      default:
        return nullptr;
    }
  }
  return nullptr;
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

  const int titleLineHeight = renderer.getTextHeight(UI_12_FONT_ID);
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

    int contentX = rowX + contentPadding;
    int rowTextWidth = rowWidth - contentPadding * 2;
    const int iconSize = hasSubtitleColumn ? mainMenuIconSize : listIconSize;

    // Draw name
    int valueWidth = 0;
    int valueTextWidth = 0;
    std::string valueText = "";
    if (rowValue != nullptr) {
      valueText = rowValue(i);
      valueText = renderer.truncatedText(UI_12_FONT_ID, valueText.c_str(), maxListValueWidth);
      valueTextWidth = renderer.getTextWidth(UI_12_FONT_ID, valueText.c_str());
      valueWidth = valueTextWidth + hPaddingInSelection;
      rowTextWidth -= valueWidth;
    }

    if (rowIcon != nullptr) {
      rowTextWidth -= iconSize + hPaddingInSelection;
      contentX += iconSize + hPaddingInSelection;
    }

    auto itemName = rowTitle(i);
    auto item = renderer.truncatedText(UI_12_FONT_ID, itemName.c_str(), rowTextWidth);
    const int titleY = itemY + contentPadding;
    renderer.drawText(UI_12_FONT_ID, contentX, titleY, item.c_str(), true);

    if (rowIcon != nullptr) {
      UIIcon icon = rowIcon(i);
      const uint8_t* iconBitmap = iconForName(icon, iconSize);
      if (iconBitmap != nullptr) {
        const int iconY = itemY + std::max(0, (rowHeight - iconSize) / 2);
        renderer.drawIcon(iconBitmap, rowX + contentPadding, iconY, iconSize, iconSize);
      }
    }

    if (rowSubtitle != nullptr) {
      std::string subtitleText = rowSubtitle(i);
      if (!subtitleText.empty()) {
        auto subtitle = renderer.truncatedText(SMALL_FONT_ID, subtitleText.c_str(), rowTextWidth);
        renderer.drawText(SMALL_FONT_ID, contentX, titleY + titleLineHeight + subtitleGap, subtitle.c_str(), true);
      }
    }

    // Draw value
    if (!valueText.empty()) {
      if (isSelected && highlightValue) {
        const int fillX = rowX + rowWidth - contentPadding - valueWidth;
        const int fillY = itemY + contentPadding / 2;
        const int fillW = valueWidth + hPaddingInSelection;
        const int fillH = rowHeight - contentPadding;

        renderer.fillRoundedRect(fillX, fillY, fillW, fillH, cornerRadius - 4, Color::LightGray);

        const int centeredTextX = fillX + (fillW - valueTextWidth) / 2;
        const int centeredTextY = fillY + (fillH - renderer.getLineHeight(UI_12_FONT_ID)) / 2;
        renderer.drawText(UI_12_FONT_ID, centeredTextX, centeredTextY, valueText.c_str(), true);
      } else {
        renderer.drawText(UI_12_FONT_ID, rowX + rowWidth - contentPadding - valueWidth, titleY, valueText.c_str(),
                          true);
      }
    }
  }
}
