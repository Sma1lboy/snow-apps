#include "snow_shot/presentation/profile/centeredtoolbarplacement.h"

#include <cstdlib>
#include <iostream>

namespace {
using snow_shot::presentation::profile::centeredToolbarPlacement;
using snow_shot::presentation::profile::CenteredToolbarRects;

void require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
        std::exit(1);
    }
}

const QRect kScreen(0, 0, 1920, 1080);
// A 400x40 main row with an option row below it (bottom) or above it (top).
const CenteredToolbarRects kBottom{QRect(0, 0, 400, 40), QRect(0, 0, 400, 84)};
const CenteredToolbarRects kTop{QRect(0, 44, 400, 40), QRect(0, 0, 400, 84)};

void centersBelowSelection() {
    const auto placement = centeredToolbarPlacement(960, 200, 600, kBottom, kTop, kScreen, 4);
    require(!placement.above, "toolbar must sit below a selection with room underneath");
    require(placement.contentPosition == QPoint(760, 605),
            "main row must be horizontally centered on the selection, gap below its bottom edge");
}

void flipsAboveWhenBottomOverflows() {
    const auto placement = centeredToolbarPlacement(960, 300, 1050, kBottom, kTop, kScreen, 4);
    require(placement.above, "toolbar must flip above when it would leave the screen bottom");
    const QRect mainRow = kTop.mainToolbar.translated(placement.contentPosition);
    require(mainRow.center().x() == 959 || mainRow.center().x() == 960,
            "flipped toolbar must stay centered");
    require(mainRow.bottom() == 300 - 4 - 1, "flipped main row must end one gap above the selection");
}

void slidesSidewaysAtScreenEdge() {
    const auto placement = centeredToolbarPlacement(30, 200, 600, kBottom, kTop, kScreen, 4);
    require(!placement.above, "a horizontal overflow alone must not flip the toolbar");
    require(placement.contentPosition.x() == 0, "toolbar must slide right to stay on screen");
    const auto right = centeredToolbarPlacement(1900, 200, 600, kBottom, kTop, kScreen, 4);
    require(right.contentPosition.x() == 1920 - 400, "toolbar must slide left to stay on screen");
}

void fullScreenSelectionKeepsBottomPreference() {
    const auto placement = centeredToolbarPlacement(960, 0, 1080, kBottom, kTop, kScreen, 4);
    require(!placement.above, "full-screen selection keeps the bottom preference");
    require(kBottom.occupied.translated(placement.contentPosition).bottom() == kScreen.bottom(),
            "full-screen selection clamps the toolbar onto the screen");
}

void invalidGeometryIsIgnored() {
    const auto placement =
        centeredToolbarPlacement(960, 200, 600, CenteredToolbarRects{}, CenteredToolbarRects{},
                                 kScreen, 4);
    require(placement.contentPosition.isNull() && !placement.above,
            "empty toolbar geometry yields a default placement");
}
} // namespace

int main() {
    centersBelowSelection();
    flipsAboveWhenBottomOverflows();
    slidesSidewaysAtScreenEdge();
    fullScreenSelectionKeepsBottomPreference();
    invalidGeometryIsIgnored();
    return 0;
}
