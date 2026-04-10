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
                                 .homeTopPadding = 40,
                                 .homeCoverHeight = 400,
                                 .homeCoverTileHeight = 400,
                                 .homeRecentBooksCount = 1,
                                 .buttonHintsHeight = 40,
                                 .sideButtonHintsWidth = 30,
                                 .progressBarHeight = 16,
                                 .progressBarMarginTop = 1,
                                 .statusBarHorizontalMargin = 5,
                                 .statusBarVerticalMargin = 19,
                                 .keyboardKeyWidth = 22,
                                 .keyboardKeyHeight = 30,
                                 .keyboardKeySpacing = 10,
                                 .keyboardBottomAligned = false,
                                 .keyboardCenteredText = false};
}

class MindfulTheme : public BaseTheme {
public:
  void drawHeader(const GfxRenderer &renderer, Rect rect, const char *title,
                  const char *subtitle) const override;
  void drawSubHeader(const GfxRenderer &renderer, Rect rect, const char *label,
                     const char *rightLabel = nullptr) const override;
  void drawButtonHints(GfxRenderer& renderer, const char* btn1, const char* btn2, const char* btn3,
                       const char* btn4) const override;
  void drawSideButtonHints(const GfxRenderer& renderer, const char* topBtn, const char* bottomBtn) const override;
  void drawTabBar(const GfxRenderer &renderer, Rect rect,
                  const std::vector<TabInfo> &tabs,
                  bool selected) const override;
  void drawList(const GfxRenderer &renderer, Rect rect, int itemCount,
                int selectedIndex,
                const std::function<std::string(int index)> &rowTitle,
                const std::function<std::string(int index)> &rowSubtitle,
                const std::function<UIIcon(int index)> &rowIcon,
                const std::function<std::string(int index)> &rowValue,
                bool highlightValue) const override;
  bool showsFileIcons() const override { return true; }
};
