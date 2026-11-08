/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameWindow.hpp
*/

#ifndef GAMEWINDOW_HPP_
#define GAMEWINDOW_HPP_

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

#include "Client/Render/Frame.hpp"
#include "Client/Settings/ClientSettings.hpp"
#include "Client/Settings/KeyBindings.hpp"
#include "Engine/Core/EventBus.hpp"
#include "Engine/Core/Vec2.hpp"
#include "Engine/Ecs/Components/UITheme.hpp"
#include "Engine/Ecs/Registry.hpp"
#include "Engine/Ecs/Systems/UIInteractionSystem.hpp"
#include "Engine/Ecs/Systems/UILayoutSystem.hpp"

namespace client::render {

/**
 * @brief The window of the game: drawing and keyboard.
 *
 * It is the only place that knows the window library. The world is drawn in
 * a fixed 1280 x 720 space, scaled to the window with black bars.
 */
class GameWindow {
public:
    /**
     * @param assetRoot Folder holding "sprites/", "fonts/" and "audio/".
     * A missing sprite or font is replaced by a plain shape or no text.
     */
    GameWindow(const settings::ClientSettings &settings,
               const std::string &title, const std::string &assetRoot);
    ~GameWindow();

    GameWindow(const GameWindow &) = delete;
    GameWindow &operator=(const GameWindow &) = delete;
    GameWindow(GameWindow &&) = delete;
    GameWindow &operator=(GameWindow &&) = delete;

    [[nodiscard]] bool isOpen() const;

    /**
     * @brief Handles the window events.
     */
    void pollEvents();

    /**
     * @brief Returns true while the physical key is held.
     */
    [[nodiscard]] static bool isKeyDown(settings::ScanCode key);

    /**
     * @brief Closes the window, which ends the game loop.
     */
    void close();

    /**
     * @brief Draws the world, then the UI layer above it.
     */
    void draw(const Frame &frame);

    /**
     * @brief Draws only the UI layer, for the menus.
     */
    void drawUi();

    /**
     * @brief The UI layer: widgets of the scenes, the HUD and the overlays.
     *
     * The window feeds the interaction system with the mouse, the typed text
     * and the keys, publishes RawKeyPressed and WindowResized on the bus,
     * and applies VideoSettingsChanged, VolumeChanged and MuteChanged.
     */
    [[nodiscard]] engine::ecs::Registry &registry() noexcept;
    [[nodiscard]] engine::core::EventBus &bus() noexcept;
    [[nodiscard]] engine::ecs::UIInteractionSystem &interaction() noexcept;
    [[nodiscard]] engine::ecs::UILayoutSystem &layout() noexcept;
    [[nodiscard]] engine::ecs::UITheme &theme() noexcept;

    /**
     * @brief Font id of the UI: an asset path, or a system font as a fallback.
     *
     * Empty when no font was found: the texts are then not drawn.
     */
    [[nodiscard]] const std::string &fontId() const noexcept;

    /** @brief Size of the window, in pixels. */
    [[nodiscard]] engine::core::Vec2 size() const;

    /** @brief Pixel of the window under a point of the 1280 x 720 world. */
    [[nodiscard]] engine::core::Vec2 toScreen(engine::core::Vec2 world) const;

    /**
     * @brief Plays a short effect, if the sound is available.
     */
    void playShot();
    void playExplosion();

private:
    class Impl;
    std::unique_ptr<Impl> _impl;
};

} /* namespace client::render */

#endif /* !GAMEWINDOW_HPP_ */
