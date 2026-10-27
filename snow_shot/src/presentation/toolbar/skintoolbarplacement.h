#ifndef SNOW_SHOT_PRESENTATION_SKINTOOLBARPLACEMENT_H
#define SNOW_SHOT_PRESENTATION_SKINTOOLBARPLACEMENT_H

#include "screenshottoolbarplacement.h"
#include "snow_shot/presentation/profile/centeredtoolbarplacement.h"
#include "snow_shot/presentation/profile/uiskin.h"

// Fork-only: routes capture toolbar placement through the active UI skin.
// The snow skin forwards to upstream's screenshotToolbarPlacement unchanged.
namespace snow_shot::presentation {
[[nodiscard]] inline ScreenshotAnchoredToolbarPlacement skinToolbarPlacement(
    const ScreenshotToolbarPresentationState& state, const ScreenshotGeometryMapper& geometry,
    const CapturedDisplayModel* display, const ScreenshotToolbarPlacementSnapshot& toolbarGeometry,
    const QRect& logicalBounds, int gap) {
    if constexpr (!profile::kMacshotSkin) {
        return screenshotToolbarPlacement(state, geometry, display, toolbarGeometry,
                                          logicalBounds, gap);
    } else {
        const ScreenshotHalfOpenRect selectionRect =
            ScreenshotHalfOpenRect::fromRect(state.selectionPixels);
        const auto logicalPosition = [&](const QPointF& point) {
            return display != nullptr
                       ? geometry.logicalPositionForCanvasPoint(*display, point).toPoint()
                       : point.toPoint();
        };
        const double centerX = (selectionRect.left + selectionRect.right) / 2.0;
        const QPoint bottomAnchor = logicalPosition(QPointF(centerX, selectionRect.bottom));
        const QPoint topAnchor = logicalPosition(QPointF(centerX, selectionRect.top));
        const auto rects = [](const ScreenshotToolbarPlacementGeometry& placement) {
            return profile::CenteredToolbarRects{placement.mainToolbarContentRect,
                                                 placement.occupiedContentRect};
        };
        const profile::CenteredToolbarPlacement placement = profile::centeredToolbarPlacement(
            bottomAnchor.x(), topAnchor.y(), bottomAnchor.y(), rects(toolbarGeometry.bottom),
            rects(toolbarGeometry.top), logicalBounds, gap);
        return ScreenshotAnchoredToolbarPlacement{placement.contentPosition, placement.above};
    }
}
} // namespace snow_shot::presentation

#endif // SNOW_SHOT_PRESENTATION_SKINTOOLBARPLACEMENT_H
