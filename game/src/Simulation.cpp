/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Simulation.cpp
*/

#include "Rtype/Simulation.hpp"

namespace rtype::game {

struct Simulation::World {
    GameRules rules;
    std::uint32_t seed = 0;
    std::uint64_t tick = 0;
    GamePhase phase = GamePhase::WAITING;
};

Simulation::Simulation(const GameRules &rules, std::uint32_t seed)
    : _world(std::make_unique<World>(World{rules, seed}))
{}

Simulation::~Simulation() = default;

Simulation::Simulation(Simulation &&) noexcept = default;

Simulation &Simulation::operator=(Simulation &&) noexcept = default;

/* to do: spawn the ship (player lifecycle task). */
bool Simulation::addPlayer(std::uint8_t)
{
    return false;
}

/* to do: destroy the ship (player lifecycle task). */
void Simulation::removePlayer(std::uint8_t)
{}

/* to do: store the keys of the player (player lifecycle task). */
void Simulation::setInput(std::uint8_t, std::uint8_t)
{}

/* to do: run the systems. */
void Simulation::step()
{
    ++_world->tick;
}

std::vector<GameEvent> Simulation::takeEvents()
{
    return {};
}

std::vector<EntityState> Simulation::states() const
{
    return {};
}

std::vector<SpawnEvent> Simulation::currentWorld() const
{
    return {};
}

std::uint64_t Simulation::tick() const noexcept
{
    return _world->tick;
}

GamePhase Simulation::phase() const noexcept
{
    return _world->phase;
}

const GameRules &Simulation::rules() const noexcept
{
    return _world->rules;
}

} /* namespace rtype::game */
