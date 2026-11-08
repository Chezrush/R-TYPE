/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** IGameDevice.hpp
*/

#ifndef IGAMEDEVICE_HPP_
#define IGAMEDEVICE_HPP_

#include "Client/Settings/KeyBindings.hpp"
#include "Engine/Core/Vec2.hpp"

namespace client::flow {

/**
 * @brief What the game needs from the machine: keys, sounds and the screen.
 *
 * The window implements it, so the game flow never knows the window library.
 */
class IGameDevice {
public:
    IGameDevice() = default;
    virtual ~IGameDevice() = default;

    IGameDevice(const IGameDevice &) = delete;
    IGameDevice &operator=(const IGameDevice &) = delete;
    IGameDevice(IGameDevice &&) = delete;
    IGameDevice &operator=(IGameDevice &&) = delete;

    /** @brief Whether the physical key is held. */
    [[nodiscard]] virtual bool isKeyDown(settings::ScanCode key) const = 0;

    virtual void playShot() = 0;
    virtual void playExplosion() = 0;

    /** @brief Pixel of the window under a point of the 1280 x 720 world. */
    [[nodiscard]] virtual engine::core::Vec2
    toScreen(engine::core::Vec2 world) const = 0;

    /** @brief Size of the window, in pixels. */
    [[nodiscard]] virtual engine::core::Vec2 size() const = 0;
};

} // namespace client::flow

#endif /* !IGAMEDEVICE_HPP_ */
