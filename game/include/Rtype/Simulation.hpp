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
     * @brief Spawns the ship of a player slot.
     *
     * @return false if the slot is already taken or not below maxPlayers.
     */
    bool addPlayer(std::uint8_t player);

    /**
     * @brief Destroys the ship of a player slot. Calling it twice is safe.
     */
    void removePlayer(std::uint8_t player);

    /**
     * @brief Sets the keys used from the next step on.
     *
     * The keys are kept until they are changed.
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

    [[nodiscard]] GamePhase phase() const noexcept;
    [[nodiscard]] const GameRules &rules() const noexcept;

private:
    /* Hides the Registry, the Scheduler and the components from the users. */
    struct World;
    std::unique_ptr<World> _world;
};

} /* namespace rtype::game */

#endif /* !SIMULATION_HPP_ */
