/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** WorldState.hpp
*/

#ifndef WORLDSTATE_HPP_
#define WORLDSTATE_HPP_

#include <array>
#include <cstdint>
#include <deque>
#include <map>
#include <string>
#include <vector>

#include "Engine/Core/Vec2.hpp"
#include "Messages.hpp"
#include "Rtype/GameRules.hpp"

namespace client::world {

/**
 * @brief What the renderer needs to draw an entity: its look and where it is.
 */
struct RenderEntity {
    std::uint16_t netId = 0;
    std::uint8_t kind = 0;
    std::uint8_t variant = 0;
    std::uint8_t owner = rtype::protocol::NO_PLAYER;
    engine::core::Vec2 position;
};

struct PlayerInfo {
    bool present = false;
    std::string name;
    std::uint32_t score = 0;
};

/**
 * @brief The game as the client knows it, built from the server messages.
 *
 * Other entities are drawn slightly in the past and interpolated between two
 * snapshots. The own ship is predicted: the keys are applied at once with
 * the rule of the server, then replayed on top of each snapshot, so the
 * controls stay responsive while the server keeps the last word. Time is
 * given in seconds by the caller, nothing here reads a clock.
 */
class WorldState {
public:
    static constexpr double INTERPOLATION_DELAY = 0.1;
    static constexpr double EXTRAPOLATION_LIMIT = 1.0;
    static constexpr double STALE_AFTER = 2.0;

    void configure(const rtype::protocol::Welcome &welcome);
    void apply(const rtype::protocol::Message &message, double now);

    /**
     * @brief Applies the keys of one step to the predicted own ship.
     */
    void predict(std::uint16_t inputId, std::uint8_t keys);

    /**
     * @brief Forgets the entities the server stopped updating.
     */
    void update(double now);

    [[nodiscard]] std::vector<RenderEntity> visible(double now) const;

    [[nodiscard]] bool configured() const noexcept;
    [[nodiscard]] std::uint8_t localSlot() const noexcept;
    [[nodiscard]] const rtype::game::GameRules &rules() const noexcept;
    [[nodiscard]] const std::array<PlayerInfo, rtype::protocol::MAX_PLAYERS> &
    players() const noexcept;
    [[nodiscard]] std::uint8_t phase() const noexcept;
    [[nodiscard]] std::uint8_t level() const noexcept;

    /**
     * @brief Lives left to the own ship: one is lost on each kill.
     */
    [[nodiscard]] unsigned lives() const noexcept;
    [[nodiscard]] std::size_t entityCount() const noexcept;

private:
    struct Sample {
        double time = 0;
        engine::core::Vec2 position;
    };

    struct Entity {
        RenderEntity look;
        engine::core::Vec2 velocity;
        double updatedAt = 0;
        std::deque<Sample> samples;
    };

    [[nodiscard]] bool isLocalShip(const Entity &entity) const noexcept;
    void applySpawn(const rtype::protocol::Spawn &spawn, double now);
    void applyDestroy(const rtype::protocol::Destroy &destroy);
    void applySnapshot(const rtype::protocol::Snapshot &snapshot, double now);
    void reconcile(const engine::core::Vec2 &serverPosition,
                   std::uint16_t inputAck);
    [[nodiscard]] static engine::core::Vec2 interpolate(const Entity &entity,
                                                        double renderTime);

    rtype::game::GameRules _rules;
    bool _configured = false;
    std::uint8_t _localSlot = 0;
    std::map<std::uint16_t, Entity> _entities;
    std::array<PlayerInfo, rtype::protocol::MAX_PLAYERS> _players;
    std::uint8_t _phase = 0;
    std::uint8_t _level = 1;
    unsigned _lives = 0;
    bool _hasPrediction = false;
    engine::core::Vec2 _predicted;
    std::deque<std::pair<std::uint16_t, std::uint8_t>> _pending;
};

} /* namespace client::world */

#endif /* !WORLDSTATE_HPP_ */
