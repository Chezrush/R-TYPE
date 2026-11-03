/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** SettingsScene.hpp
*/

#ifndef SETTINGSSCENE_HPP_
#define SETTINGSSCENE_HPP_

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "Client/Scenes/SceneRoute.hpp"
#include "Client/Settings/SettingsStore.hpp"
#include "Engine/Ecs/Components/UIWidgetEvents.hpp"
#include "Engine/Ecs/Systems/UILayoutSystem.hpp"
#include "Engine/Ecs/UIOverlay.hpp"

namespace client::scenes {

/** @brief The controls of the settings screen, in focus order. */
enum class SettingsControl : std::uint8_t {
    MASTER_VOLUME,
    MUSIC_VOLUME,
    SFX_VOLUME,
    MUTE,
    FULLSCREEN,
    RESOLUTION,
    VSYNC,
    UI_SCALE,
    PALETTE,
    HIGH_CONTRAST,
    REDUCED_MOTION,
    RESET,
    BACK
};

/**
 * @brief Settings screen, shown above the main menu or the in-game menu.
 *
 * It edits the values and publishes the changes (VolumeChanged, MuteChanged,
 * VideoSettingsChanged): the audio and window code apply them. The UI scale and
 * the theme are applied here, directly. Back or Escape return to the scene
 * below. The file is written when the scene exits, so the store must outlive
 * the scene.
 */
class SettingsScene : public engine::ecs::UIOverlay {
public:
    SettingsScene(engine::ecs::UISceneContext context,
                  settings::SettingsStore &store,
                  engine::ecs::UILayoutSystem &layout,
                  engine::ecs::UITheme &theme);

    void onExit() override;

    [[nodiscard]] engine::ecs::Entity control(SettingsControl control) const;

protected:
    void activate() override;

private:
    void buildTitles();
    void buildAudio();
    void buildVideo();
    void buildAccessibility();
    void buildButtons();
    engine::ecs::Entity addRowLabel(const char *text, float column, float row);
    void addVolumeRow(const char *action, float row);
    void addToggleRow(const char *text, const char *action, float column,
                      float row);
    void addDropdownRow(const char *text, const char *action, float column,
                        float row, std::vector<std::string> options);
    void listenToWidgets();
    void onValue(const engine::ecs::components::UIValueChangedEvent &event);
    void onVolume(events::VolumeBus bus, int value);
    void onToggle(const std::string &action, bool checked);
    void
    onSelection(const engine::ecs::components::UISelectionChangedEvent &event);
    void onButton(const engine::ecs::components::UIButtonClickedEvent &event);
    void reset();
    void syncWidgets();
    void syncVolumes();
    void syncVolumeLabels();
    void syncVideo();
    void syncAccessibility();
    void publishVolumes();
    void publishVideo();
    void applyUIScale();
    void applyTheme();

    settings::SettingsStore &_store;
    engine::ecs::UILayoutSystem &_layout;
    engine::ecs::UITheme &_theme;
    std::vector<engine::ecs::Entity> _controls;
    std::vector<engine::ecs::Entity> _volumeLabels;
};

/** @brief Gives the router the settings screen. */
void addSettingsRoute(SceneRouter &router,
                      const engine::ecs::UISceneContext &context,
                      settings::SettingsStore &store,
                      engine::ecs::UILayoutSystem &layout,
                      engine::ecs::UITheme &theme);

} // namespace client::scenes

#endif /* !SETTINGSSCENE_HPP_ */
