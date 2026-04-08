#pragma once

#include "components/themes/BaseTheme.h"

class GfxRenderer;

// Mindful theme metrics (zero runtime cost)
namespace MindfulMetrics {
constexpr ThemeMetrics values = {.batteryWidth = 15,
                                 .batteryHeight = 12,
                                 .topPadding = 5,
                                 .batteryBarHeight = 20,
                                 .headerHeight = 45,
                                 .verticalSpacing = 10,
                                 .contentSidePadding = 20,
                                 .listRowHeight = 30,
                                 .listWithSubtitleRowHeight = 65,
                                 .menuRowHeight = 45,
                                 .menuSpacing = 8,
                                 .tabSpacing = 10,
                                 .tabBarHeight = 50,
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
  // Mindful theme inherits most functionality from BaseTheme
  // Override methods here if you want to customize specific UI elements

  // Example: You can override specific methods like this:
  // void drawHeader(const GfxRenderer& renderer, Rect rect, const char* title, const char* subtitle) const override;
  // void drawBatteryLeft(const GfxRenderer& renderer, Rect rect, bool showPercentage = true) const override;

  // For now, this theme uses the base implementation but with custom metrics
};
