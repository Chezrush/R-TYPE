/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameContext.hpp
*/

#ifndef GAMECONTEXT_HPP_
#define GAMECONTEXT_HPP_

#include <vector>

#include "Engine/Ecs/Entity.hpp"
#include "Rtype/GameEvents.hpp"
#include "Rtype/GameRules.hpp"
#include "Rtype/NetIdAllocator.hpp"
#include "Rtype/PlayerState.hpp"
#include "Rtype/Random.hpp"
#include "Rtype/Waves.hpp"

namespace rtype::game {

/**
 * @brief Two entities whose hitboxes overlap, from opposite teams.
 */
struct Hit {
    engine::ecs::Entity first;
    engine::ecs::Entity second;
};

/**
 * @brief State shared by the systems and the entity factories of one game.
 *
 * It is not part of the ECS: it holds what is not a component, the rules,
 * the net_id in use, the random numbers, the progress of the waves, the
 * players and the events nobody has taken yet.
 */
struct GameContext {
    GameRules rules;
    NetIdAllocator netIds{};
    std::vector<GameEvent> events{};
    Random random{};
    WaveState wave{};
    GamePhase phase = GamePhase::WAITING;
    std::uint8_t level = 1;
    std::vector<PlayerState> players{};
    std::vector<Hit> hits{};
};

} /* namespace rtype::game */

#endif /* !GAMECONTEXT_HPP_ */
