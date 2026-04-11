#pragma once

#include "components/themes/BaseTheme.h"

class GfxRenderer;

// Mindful theme metrics (zero runtime cost)
namespace MindfulMetrics {
constexpr ThemeMetrics values = {.batteryWidth = 16,
                                 .batteryHeight = 12,
                                 .topPadding = 5,
                                 .batteryBarHeight = 40,
                                 .headerHeight = 84,
                                 .verticalSpacing = 20,
                                 .contentSidePadding = 20,
                                 .listRowHeight = 30,
                                 .listWithSubtitleRowHeight = 65,
                                 .menuRowHeight = 45,
                                 .menuSpacing = 8,
                                 .tabSpacing = 10,
                                 .tabBarHeight = 40,
                                 .scrollBarWidth = 4,
                                 .scrollBarRightOffset = 5,
                                 .homeTopPadding = 25,
                                 .homeCoverHeight = 400,
                                 .homeCoverTileHeight = 500,
                                 .homeRecentBooksCount = 3,
                                 .buttonHintsHeight = 40,
                                 .sideButtonHintsWidth = 30,
                                 .progressBarHeight = 16,
                                 .progressBarMarginTop = 1,
                                 .statusBarHorizontalMargin = 5,
                                 .statusBarVerticalMargin = 19,
                                 .keyboardKeyWidth = 33,
                                 .keyboardKeyHeight = 50,
                                 .keyboardKeySpacing = 1,
                                 .keyboardBottomAligned = true,
                                 .keyboardCenteredText = true};

// Recent carousel metrics
static constexpr int recentCarouselActiveCoverHeight = 360;
static constexpr int recentCarouselActiveCoverWidth = (recentCarouselActiveCoverHeight * 2) / 3;
static constexpr int recentCarouselActiveBorderWidth = 2;
static constexpr int recentCarouselSideCoverHeight = 280;
static constexpr int recentCarouselSideCoverWidth = 188;
static constexpr int recentCarouselHorizontalGap = 16;
static constexpr int recentCarouselTopOffset = 10;
static constexpr int recentCarouselTextSlotWidth = 320;
static constexpr int recentCarouselTitleHeight = 24;
static constexpr int recentCarouselAuthorHeight = 24;
static constexpr int recentCarouselGapCoverToTitle = 12;
static constexpr int recentCarouselGapTitleToAuthor = 4;
static constexpr int recentCarouselGapAuthorToDots = 12;
static constexpr int recentCarouselDotSize = 8;
static constexpr int recentCarouselDotGap = 10;
static constexpr int recentCarouselMaxVisibleDots = 5;
static constexpr int recentCarouselPlaceholderIconSize = 32;
}

class MindfulTheme : public BaseTheme {
 public:
  void drawHeader(const GfxRenderer& renderer, Rect rect, const char* title, const char* subtitle) const override;
  void drawSubHeader(const GfxRenderer& renderer, Rect rect, const char* label,
                     const char* rightLabel = nullptr) const override;
  void drawRecentBookCover(GfxRenderer& renderer, Rect rect, const std::vector<RecentBook>& recentBooks,
                           const int selectorIndex, bool& coverRendered, bool& coverBufferStored, bool& bufferRestored,
                           std::function<bool()> storeCoverBuffer) const override;
  void drawButtonHints(GfxRenderer& renderer, const char* btn1, const char* btn2, const char* btn3,
                       const char* btn4) const override;
  void drawSideButtonHints(const GfxRenderer& renderer, const char* topBtn, const char* bottomBtn) const override;
  void drawTabBar(const GfxRenderer& renderer, Rect rect, const std::vector<TabInfo>& tabs,
                  bool selected) const override;
  void drawList(const GfxRenderer& renderer, Rect rect, int itemCount, int selectedIndex,
                const std::function<std::string(int index)>& rowTitle,
                const std::function<std::string(int index)>& rowSubtitle,
                const std::function<UIIcon(int index)>& rowIcon, const std::function<std::string(int index)>& rowValue,
                bool highlightValue) const override;
  void drawKeyboardKey(const GfxRenderer& renderer, Rect rect, const char* label, const bool isSelected) const override;
  void drawButtonMenu(GfxRenderer& renderer, Rect rect, int buttonCount, int selectedIndex,
                      const std::function<std::string(int index)>& buttonLabel,
                      const std::function<UIIcon(int index)>& rowIcon) const override;
  bool showsFileIcons() const override { return true; }
};
