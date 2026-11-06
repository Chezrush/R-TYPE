/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** DamageSystem.cpp
*/

#include "Rtype/Systems/DamageSystem.hpp"

#include <algorithm>
#include <set>

#include "Rtype/Components/Components.hpp"
#include "Rtype/Entities.hpp"
#include "Rtype/Players.hpp"

namespace rtype::game {

namespace {

struct Combat {
    engine::ecs::Registry &registry;
    engine::ecs::CommandBuffer &commands;
    GameContext &context;
    std::set<std::uint32_t> dead;
};

std::uint8_t ownerOf(const Combat &combat, engine::ecs::Entity entity)
{
    const Kind *kind = combat.registry.getComponents<Kind>().get(entity);

    if (kind == nullptr) {
        return NO_PLAYER;
    }
    return kind->owner;
}

void destroy(Combat &combat, engine::ecs::Entity victim, std::uint8_t killer)
{
    const NetId *netId = combat.registry.getComponents<NetId>().get(victim);
    const Kind *kind = combat.registry.getComponents<Kind>().get(victim);

    if (netId == nullptr || kind == nullptr) {
        return;
    }
    announceDestroy(combat.context, netId->value, DestroyReason::KILLED,
                    killer);
    combat.commands.kill(victim);
    if (kind->kind == EntityKind::PLAYER_SHIP) {
        onShipDestroyed(combat.context, kind->owner);
    } else if (kind->kind == EntityKind::BYDOS) {
        addScore(combat.context, killer, combat.context.rules.enemyScore);
    }
}

void strike(Combat &combat, engine::ecs::Entity target,
            engine::ecs::Entity source)
{
    Health *health = combat.registry.getComponents<Health>().get(target);

    if (health == nullptr) {
        return;
    }
    health->current = std::max(0, health->current - 1);
    if (health->current == 0) {
        combat.dead.insert(target.index());
        destroy(combat, target, ownerOf(combat, source));
    }
}

void resolve(Combat &combat, const Hit &hit)
{
    if (combat.dead.contains(hit.first.index()) ||
        combat.dead.contains(hit.second.index())) {
        return;
    }
    strike(combat, hit.first, hit.second);
    strike(combat, hit.second, hit.first);
}

} // namespace

void addDamageSystem(engine::ecs::Scheduler &scheduler, GameContext &context)
{
    scheduler.addSystem(
        "damage", engine::ecs::Phase::UPDATE, engine::ecs::Access{},
        [&context](engine::ecs::Registry &registry,
                   engine::ecs::CommandBuffer &commands, double) {
            Combat combat{registry, commands, context, {}};

            for (const Hit &hit : context.hits) {
                resolve(combat, hit);
            }
        });
}

} /* namespace rtype::game */
