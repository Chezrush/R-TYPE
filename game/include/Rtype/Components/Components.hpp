/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Components.hpp
*/

#ifndef COMPONENTS_HPP_
#define COMPONENTS_HPP_

#include "Engine/Ecs/Registry.hpp"
#include "Rtype/Components/EnemyAI.hpp"
#include "Rtype/Components/Health.hpp"
#include "Rtype/Components/Hitbox.hpp"
#include "Rtype/Components/Kind.hpp"
#include "Rtype/Components/Lives.hpp"
#include "Rtype/Components/NetId.hpp"
#include "Rtype/Components/PlayerControl.hpp"
#include "Rtype/Components/Position.hpp"
#include "Rtype/Components/Team.hpp"
#include "Rtype/Components/Velocity.hpp"

namespace rtype::game {

/**
 * @brief Registers every R-Type component in the registry.
 *
 * Registering twice is safe: the existing storages are kept.
 */
void registerComponents(engine::ecs::Registry &registry);

} /* namespace rtype::game */

#endif /* !COMPONENTS_HPP_ */
