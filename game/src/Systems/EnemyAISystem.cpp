/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** EnemyAISystem.cpp
*/

#include "Rtype/Systems/EnemyAISystem.hpp"

#include <cmath>

#include "Rtype/Components/Components.hpp"
#include "Rtype/Entities.hpp"

namespace rtype::game {

namespace {

constexpr float WAVE_SPEED = 120.0F;
constexpr float WAVE_FREQUENCY = 2.0F;

float verticalSpeed(const EnemyAI &ai) noexcept
{
    if (ai.pattern != EnemyPattern::SINE_WAVE) {
        return 0.0F;
    }
    return WAVE_SPEED * std::cos(WAVE_FREQUENCY * ai.age);
}

bool readyToShoot(const EnemyAI &ai, const Position &position,
                  const GameRules &rules) noexcept
{
    return ai.fireCooldown <= 0.0F && position.value.x <= rules.worldWidth;
}

} // namespace

void addEnemyAISystem(engine::ecs::Scheduler &scheduler,
                      engine::ecs::Registry &registry, GameContext &context)
{
    scheduler.addSystem<EnemyAI, Velocity, const Position>(
        "enemy-ai", engine::ecs::Phase::UPDATE,
        [&registry, &context](engine::ecs::Entity, EnemyAI &ai,
                              Velocity &velocity, const Position &position,
                              engine::ecs::CommandBuffer &commands,
                              double deltaTime) {
            ai.age += static_cast<float>(deltaTime);
            ai.fireCooldown -= static_cast<float>(deltaTime);
            velocity.value.y = verticalSpeed(ai);
            if (readyToShoot(ai, position, context.rules) &&
                spawnEnemyMissile(registry, commands, context, position)) {
                ai.fireCooldown = context.rules.enemyFireInterval;
            }
        });
}

} /* namespace rtype::game */
