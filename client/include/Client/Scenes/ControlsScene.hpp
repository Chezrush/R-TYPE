/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ControlsScene.hpp
*/

#ifndef CONTROLSSCENE_HPP_
#define CONTROLSSCENE_HPP_

#include <optional>
#include <vector>

#include "Client/Events/SettingsEvents.hpp"
#include "Client/Scenes/SceneRoute.hpp"
#include "Client/Settings/SettingsStore.hpp"
#include "Engine/Ecs/Components/UIWidgetEvents.hpp"
#include "Engine/Ecs/UIOverlay.hpp"

namespace client::scenes {

/**
 * @brief Screen to change the controls, above the main or the in-game menu.
 *
 * Activating an action asks for a key or a gamepad button. While it waits, the
 * UI input is discarded and the scene reads RawKeyPressed and
 * RawGamepadButtonPressed, which the window code publishes before it calls
 * handleInput(). Escape cancels, except for "Open menu", which can use it. A
 * conflict only shows a warning: the player may want it for a moment. Every
 * change publishes BindingsChanged. The file is written when the scene exits,
 * so the store must outlive the scene.
 */
class ControlsScene : public engine::ecs::UIOverlay {
public:
    ControlsScene(engine::ecs::UISceneContext context,
                  settings::SettingsStore &store);

    void onExit() override;
    void handleInput() override;

    [[nodiscard]] engine::ecs::Entity
    actionButton(settings::GameAction action) const;
    [[nodiscard]] engine::ecs::Entity autoFireToggle() const;
    [[nodiscard]] engine::ecs::Entity beamDropdown() const;
    [[nodiscard]] engine::ecs::Entity resetButton() const;
    [[nodiscard]] engine::ecs::Entity backButton() const;
    [[nodiscard]] engine::ecs::Entity warningLabel() const;
    [[nodiscard]] engine::ecs::Entity captureLabel() const;
    [[nodiscard]] bool isCapturing() const noexcept;

protected:
    void activate() override;

private:
    void buildActions();
    void buildOptions();
    void buildButtons();
    void buildCaptureModal();
    void listenToWidgets();
    void listenToDevices();
    void onButton(const engine::ecs::components::UIButtonClickedEvent &event);
    void onKey(settings::ScanCode key);
    void onGamepadButton(settings::GamepadButton button);
    void startCapture(settings::GameAction action);
    void stopCapture();
    void reset();
    void changed();
    void syncWidgets();

    settings::SettingsStore &_store;
    std::vector<engine::ecs::Entity> _rows;
    std::vector<engine::ecs::Entity> _parts;
    std::optional<settings::GameAction> _capturing;
    bool _discardNextInput = false;
};

/** @brief Gives the router the controls screen. */
void addControlsRoute(SceneRouter &router,
                      const engine::ecs::UISceneContext &context,
                      settings::SettingsStore &store);

} // namespace client::scenes

#endif /* !CONTROLSSCENE_HPP_ */
