#ifndef SNOW_SHOT_PRESENTATION_PROFILE_CENTEREDTOOLBARPLACEMENT_H
#define SNOW_SHOT_PRESENTATION_PROFILE_CENTEREDTOOLBARPLACEMENT_H

#include <QPoint>
#include <QRect>

#include <algorithm>

// macshot-style toolbar placement: the main row is horizontally centered on
// the selection, below it when it fits, otherwise above it. Kept free of
// presentation types so it can be unit tested with QtCore alone.
namespace snow_shot::presentation::profile {
// Rects are in toolbar content coordinates, as in
// ScreenshotToolbarPlacementGeometry.
struct CenteredToolbarRects {
    QRect mainToolbar;
    QRect occupied;

    [[nodiscard]] bool isValid() const { return !mainToolbar.isEmpty(); }
    [[nodiscard]] QRect occupiedOrMain() const {
        return occupied.isEmpty() ? mainToolbar : occupied;
    }
};

struct CenteredToolbarPlacement {
    QPoint contentPosition;
    bool above = false;
};

[[nodiscard]] inline bool centeredToolbarFits(const QRect& occupied, const QPoint& position,
                                              const QRect& bounds) {
    if (occupied.isEmpty() || !bounds.isValid() || bounds.isEmpty()) {
        return false;
    }
    return bounds.contains(occupied.translated(position));
}

[[nodiscard]] inline QPoint clampCenteredToolbar(const QPoint& position, const QRect& occupied,
                                                 const QRect& bounds) {
    if (occupied.isEmpty() || !bounds.isValid() || bounds.isEmpty()) {
        return position;
    }
    const int minX = bounds.left() - occupied.left();
    const int maxX = std::max(minX, bounds.right() - occupied.right());
    const int minY = bounds.top() - occupied.top();
    const int maxY = std::max(minY, bounds.bottom() - occupied.bottom());
    return {std::clamp(position.x(), minX, maxX), std::clamp(position.y(), minY, maxY)};
}

// selectionCenterX, selectionTop and selectionBottom are logical coordinates;
// selectionBottom is the exclusive bottom edge, matching anchoredToolbarPlacement.
[[nodiscard]] inline CenteredToolbarPlacement
centeredToolbarPlacement(int selectionCenterX, int selectionTop, int selectionBottom,
                         const CenteredToolbarRects& bottomRects,
                         const CenteredToolbarRects& topRects, const QRect& bounds, int gap) {
    if (!bottomRects.isValid() && !topRects.isValid()) {
        return {};
    }
    const int effectiveGap = std::max(0, gap);
    const auto centeredX = [selectionCenterX](const QRect& main) {
        return selectionCenterX - (main.left() + main.width() / 2);
    };

    QPoint bottomPosition;
    if (bottomRects.isValid()) {
        bottomPosition = QPoint(centeredX(bottomRects.mainToolbar),
                                selectionBottom + effectiveGap + 1 - bottomRects.mainToolbar.top());
        // Sliding sideways to stay on screen is fine; only vertical overflow flips sides.
        const QPoint clamped =
            clampCenteredToolbar(bottomPosition, bottomRects.occupiedOrMain(), bounds);
        if (centeredToolbarFits(bottomRects.occupiedOrMain(),
                                QPoint(clamped.x(), bottomPosition.y()), bounds)) {
            return {clamped, false};
        }
    }
    if (topRects.isValid()) {
        const QPoint topPosition(centeredX(topRects.mainToolbar),
                                 selectionTop - topRects.mainToolbar.bottom() - effectiveGap - 1);
        const QPoint clamped = clampCenteredToolbar(topPosition, topRects.occupiedOrMain(), bounds);
        if (centeredToolbarFits(topRects.occupiedOrMain(), QPoint(clamped.x(), topPosition.y()),
                                bounds)) {
            return {clamped, true};
        }
    }
    // Neither side fits (selection fills the screen): keep the bottom preference and
    // clamp, so the toolbar overlaps the selection's bottom edge like macshot does.
    if (bottomRects.isValid()) {
        return {clampCenteredToolbar(bottomPosition, bottomRects.occupiedOrMain(), bounds), false};
    }
    const QPoint topPosition(centeredX(topRects.mainToolbar),
                             selectionTop - topRects.mainToolbar.bottom() - effectiveGap - 1);
    return {clampCenteredToolbar(topPosition, topRects.occupiedOrMain(), bounds), true};
}
} // namespace snow_shot::presentation::profile

#endif // SNOW_SHOT_PRESENTATION_PROFILE_CENTEREDTOOLBARPLACEMENT_H
