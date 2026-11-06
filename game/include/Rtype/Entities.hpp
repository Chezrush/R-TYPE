/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Entities.hpp
*/

#ifndef ENTITIES_HPP_
#define ENTITIES_HPP_

#include <cstdint>
#include <optional>
#include <vector>

#include "Engine/Ecs/CommandBuffer.hpp"
#include "Engine/Ecs/Entity.hpp"
#include "Engine/Ecs/Registry.hpp"
#include "Rtype/Components/Components.hpp"
#include "Rtype/GameContext.hpp"

namespace rtype::game {

inline constexpr float SHIP_WIDTH = 64.0F;
inline constexpr float SHIP_HEIGHT = 32.0F;
inline constexpr float MISSILE_WIDTH = 24.0F;
inline constexpr float MISSILE_HEIGHT = 8.0F;
inline constexpr float ENEMY_WIDTH = 48.0F;
inline constexpr float ENEMY_HEIGHT = 48.0F;

/**
 * @brief Creates the ship of a player slot and tells it with a SpawnEvent.
 *
 * @return The ship, or std::nullopt if no net_id is free.
 */
[[nodiscard]] std::optional<engine::ecs::Entity>
spawnShip(engine::ecs::Registry &registry, GameContext &context,
          std::uint8_t slot);

/**
 * @brief Records the creation of a player missile in front of a ship.
 *
 * The SpawnEvent is emitted now and the missile exists once the command
 * buffer is flushed.
 *
 * @return false if no net_id is free, nothing is recorded in that case.
 */
bool spawnMissile(engine::ecs::Registry &registry,
                  engine::ecs::CommandBuffer &commands, GameContext &context,
                  const Position &shipPosition, std::uint8_t owner);

/**
 * @brief Records the creation of an enemy just outside the right edge.
 *
 * Its height and its pattern come from the random numbers of the context,
 * its speed from the current level. The SpawnEvent is emitted now and the
 * enemy exists once the command buffer is flushed.
 *
 * @return false if no net_id is free, nothing is recorded in that case.
 */
bool spawnEnemy(engine::ecs::Registry &registry,
                engine::ecs::CommandBuffer &commands, GameContext &context);

/**
 * @brief Records the creation of an enemy missile in front of an enemy.
 *
 * @return false if no net_id is free, nothing is recorded in that case.
 */
bool spawnEnemyMissile(engine::ecs::Registry &registry,
                       engine::ecs::CommandBuffer &commands,
                       GameContext &context, const Position &enemyPosition);

/** @brief Whether at least one player ship is alive. */
[[nodiscard]] bool hasPlayers(const engine::ecs::Registry &registry);

/**
 * @brief Frees a net_id and emits the DestroyEvent of its entity.
 *
 * It does not touch the registry: a system that iterates over the components
 * calls it, then records the kill in its command buffer.
 */
void announceDestroy(GameContext &context, std::uint16_t netId,
                     DestroyReason reason, std::uint8_t killer);

/**
 * @brief Destroys an entity, frees its net_id and emits a DestroyEvent.
 *
 * A stale handle does nothing.
 */
void destroyEntity(engine::ecs::Registry &registry, GameContext &context,
                   engine::ecs::Entity entity, DestroyReason reason,
                   std::uint8_t killer);

/** @brief Position of every living networked entity, for snapshots. */
[[nodiscard]] std::vector<EntityState>
collectStates(const engine::ecs::Registry &registry);

/** @brief A spawn for every living networked entity. */
[[nodiscard]] std::vector<SpawnEvent>
collectSpawns(const engine::ecs::Registry &registry);

} /* namespace rtype::game */

#endif /* !ENTITIES_HPP_ */
