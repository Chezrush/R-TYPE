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

#include "Client/Hud/HudState.hpp"
#include "Client/Scenes/Starfield.hpp"
#include "Client/Settings/ClientSettings.hpp"
#include "Client/Settings/KeyBindings.hpp"
#include "Client/World/WorldState.hpp"

namespace client::render {

/**
 * @brief What the window draws in one frame.
 */
struct Frame {
    const world::WorldState &world;
    const scenes::Starfield &stars;
    double now = 0;
    std::string banner;
    int rttMs = 0;
    settings::ColorPalette palette = settings::ColorPalette::NORMAL;
    bool highContrast = false;
    const hud::HudState *hud = nullptr;
    std::array<std::string, 4> cues{};
};

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

    void draw(const Frame &frame);

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
