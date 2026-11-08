/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** CollisionSystem.cpp
*/

#include "Rtype/Systems/CollisionSystem.hpp"

#include "Engine/Ecs/IndexedZipper.hpp"
#include "Rtype/Collision.hpp"
#include "Rtype/Components/Components.hpp"

namespace rtype::game {

namespace {

struct Body {
    engine::ecs::Entity entity;
    engine::core::Vec2 position;
    Hitbox box;
    TeamSide side;
};

std::vector<Body> collectBodies(const engine::ecs::Registry &registry)
{
    std::vector<Body> bodies;

    for (auto row :
         engine::ecs::makeIndexedZipper(registry.getComponents<Position>(),
                                        registry.getComponents<Hitbox>(),
                                        registry.getComponents<Team>())) {
        const auto &[index, position, box, team] = row;
        bodies.push_back(
            {registry.entityAt(index), position.value, box, team.side});
    }
    return bodies;
}

bool collide(const Body &first, const Body &second) noexcept
{
    return first.side != second.side &&
           boxesOverlap(first.position, first.box, second.position, second.box);
}

void findHitsWith(const std::vector<Body> &bodies, std::size_t first,
                  std::vector<Hit> &hits)
{
    for (std::size_t other = first + 1; other < bodies.size(); ++other) {
        if (collide(bodies[first], bodies[other])) {
            hits.push_back({bodies[first].entity, bodies[other].entity});
        }
    }
}

void findHits(const std::vector<Body> &bodies, std::vector<Hit> &hits)
{
    for (std::size_t first = 0; first < bodies.size(); ++first) {
        findHitsWith(bodies, first, hits);
    }
}

} // namespace

void addCollisionSystem(engine::ecs::Scheduler &scheduler, GameContext &context)
{
    scheduler.addSystem("collision", engine::ecs::Phase::UPDATE,
                        engine::ecs::Access{},
                        [&context](engine::ecs::Registry &registry,
                                   engine::ecs::CommandBuffer &, double) {
                            context.hits.clear();
                            findHits(collectBodies(registry), context.hits);
                        });
}

} /* namespace rtype::game */
