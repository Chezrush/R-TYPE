/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** BeamGauge.hpp
*/

#ifndef BEAMGAUGE_HPP_
#define BEAMGAUGE_HPP_

namespace client::hud {

/**
 * @brief Beam charge, driven by the local shoot key.
 *
 * Empty when idle, fills while shoot is held (full after chargeSeconds),
 * back to empty as soon as shoot is released.
 */
class BeamGauge {
public:
    explicit BeamGauge(float chargeSeconds = 1.5f) noexcept;

    /** @brief Advances the charge by dt seconds. */
    void update(float dt, bool shootHeld) noexcept;

    /** @brief Empties the gauge. */
    void reset() noexcept;

    /** @brief Charge in [0, 1]. */
    [[nodiscard]] float charge() const noexcept;

    [[nodiscard]] bool isFull() const noexcept;

private:
    float _chargeSeconds;
    float _charge = 0.0f;
};

} // namespace client::hud

#endif /* !BEAMGAUGE_HPP_ */
