#ifndef SNOW_SHOT_PRESENTATION_PROFILE_UISKIN_H
#define SNOW_SHOT_PRESENTATION_PROFILE_UISKIN_H

// Fork-only UI skin switch, selected at configure time by SNOW_SHOT_UI_SKIN
// (cmake/SnowShotUiSkin.cmake). Upstream files reach the skin only through
// hooks marked FORK(macshot).
namespace snow_shot::presentation::profile {
enum class UiSkin { Snow, Macshot };

#if defined(SNOW_SHOT_UI_SKIN_MACSHOT) && SNOW_SHOT_UI_SKIN_MACSHOT
inline constexpr UiSkin kActiveUiSkin = UiSkin::Macshot;
#else
inline constexpr UiSkin kActiveUiSkin = UiSkin::Snow;
#endif

inline constexpr bool kMacshotSkin = kActiveUiSkin == UiSkin::Macshot;
} // namespace snow_shot::presentation::profile

#endif // SNOW_SHOT_PRESENTATION_PROFILE_UISKIN_H
