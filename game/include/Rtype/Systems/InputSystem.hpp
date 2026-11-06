/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** InputSystem.hpp
*/

#ifndef INPUTSYSTEM_HPP_
#define INPUTSYSTEM_HPP_

#include "Engine/Ecs/Registry.hpp"
#include "Engine/Ecs/Scheduler.hpp"
#include "Rtype/GameContext.hpp"

namespace rtype::game {

/**
 * @brief Adds the input system, run before every other system of a tick.
 *
 * For each player ship it turns the held keys into a velocity, counts the
 * fire cooldown down, and shoots a missile if the shoot key is held and the
 * cooldown is over. The registry and the context must outlive the scheduler.
 */
void addInputSystem(engine::ecs::Scheduler &scheduler,
                    engine::ecs::Registry &registry, GameContext &context);

} /* namespace rtype::game */

#endif /* !INPUTSYSTEM_HPP_ */
