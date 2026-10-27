#ifndef SNOW_SHOT_PRESENTATION_PROFILE_MACSHOTTHEME_H
#define SNOW_SHOT_PRESENTATION_PROFILE_MACSHOTTHEME_H

#include "theme/theme_types.h"

#include <QColor>

// macshot-style capture toolbar look: a dark translucent pill with white
// icons and a system-blue accent, regardless of the app's light/dark mode.
namespace snow_shot::presentation::profile::macshot {
inline constexpr int kPanelRadius = 12;

[[nodiscard]] inline QColor toolbarSurfaceColor() {
    return QColor(30, 30, 32, 235);
}

[[nodiscard]] inline QColor toolbarSeparatorColor() {
    return QColor(255, 255, 255, 38);
}

[[nodiscard]] inline QColor toolbarDragHandleColor() {
    return QColor(255, 255, 255, 89);
}

// Scoped to each toolbar row so controls inside it render dark without
// changing the rest of the app.
[[nodiscard]] inline adqt::theme::ThemeOverride toolbarThemeOverride() {
    adqt::theme::ThemeOverride value;
    value.scheme = adqt::theme::ThemeScheme::Dark;
    value.primary = QColor(0x0A, 0x84, 0xFF);
    value.borderRadius = 8.0;
    return value;
}
} // namespace snow_shot::presentation::profile::macshot

#endif // SNOW_SHOT_PRESENTATION_PROFILE_MACSHOTTHEME_H
