/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Simulation.hpp
*/

#ifndef SIMULATION_HPP_
#define SIMULATION_HPP_

#include <cstdint>
#include <memory>
#include <vector>

#include "Rtype/GameEvents.hpp"
#include "Rtype/GameRules.hpp"

namespace rtype::game {

/**
 * @brief The R-Type world, advanced one fixed tick at a time.
 *
 * Knows nothing about the network: players are slot indexes and inputs are
 * the keys bitmask (see rtype::game::keys). The world never reads the
 * clock, so the same rules, seed and inputs always give the same game.
 *
 * Not thread-safe: it is owned by the simulation thread of the server.
 */
class Simulation {
public:
    /**
     * @param rules Rules of the game, kept for its whole life.
     * @param seed Seed of every random choice (enemy waves, patterns).
     */
    Simulation(const GameRules &rules, std::uint32_t seed);
    ~Simulation();

    Simulation(const Simulation &) = delete;
    Simulation &operator=(const Simulation &) = delete;

    Simulation(Simulation &&) noexcept;
    Simulation &operator=(Simulation &&) noexcept;

    /**
     * @brief Spawns the ship of a player slot and emits its SpawnEvent.
     *
     * @return false if the slot is already taken, not below maxPlayers, or
     * if no net_id is free.
     */
    bool addPlayer(std::uint8_t player);

    /**
     * @brief Destroys the ship of a player slot and emits a DestroyEvent.
     *
     * The reason is LEFT_SCREEN (expired) and there is no killer. A slot
     * without a ship is ignored, so calling it twice is safe.
     */
    void removePlayer(std::uint8_t player);

    /**
     * @brief Sets the keys used from the next step on.
     *
     * The keys are kept until they are changed, and a player who is waiting
     * to respawn gets them back with the new ship. Bits that are not in
     * keys::MASK are ignored, and so is a slot without a player.
     */
    void setInput(std::uint8_t player, std::uint8_t keys);

    /**
     * @brief Advances the world by exactly 1 / tickRate second.
     */
    void step();

    /**
     * @brief Returns the events since the last call, in the order they
     * happened, and clears them.
     */
    [[nodiscard]] std::vector<GameEvent> takeEvents();

    /**
     * @brief Returns the position of every living entity, for snapshots.
     */
    [[nodiscard]] std::vector<EntityState> states() const;

    /**
     * @brief Returns a spawn for every living entity, for a client joining
     * late.
     */
    [[nodiscard]] std::vector<SpawnEvent> currentWorld() const;

    /**
     * @brief Returns the number of steps performed since the creation.
     */
    [[nodiscard]] std::uint64_t tick() const noexcept;

    /**
     * @brief Returns the phase of the game.
     *
     * WAITING while nobody is in the game, PLAYING from the tick after the
     * first player joins, GAME_OVER when every player is out of lives. After
     * rules.gameOverDelay seconds of GAME_OVER a new game starts with the
     * players still there (everything is reset), or the phase goes back to
     * WAITING if nobody is left. Every change is reported by a GameStateEvent.
     */
    [[nodiscard]] GamePhase phase() const noexcept;

    /** @brief Returns the current level, 1 at the start of a game. */
    [[nodiscard]] std::uint8_t level() const noexcept;

    [[nodiscard]] const GameRules &rules() const noexcept;

private:
    /* Hides the Registry, the Scheduler and the components from the users. */
    struct World;
    std::unique_ptr<World> _world;
};

} /* namespace rtype::game */

#endif /* !SIMULATION_HPP_ */
