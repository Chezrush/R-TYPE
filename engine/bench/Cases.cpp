/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Cases.cpp
*/

#include <vector>

#include "Bench.hpp"
#include "Engine/Ecs/CommandBuffer.hpp"
#include "Engine/Ecs/Registry.hpp"
#include "Engine/Ecs/Scheduler.hpp"
#include "Engine/Ecs/Zipper.hpp"

using engine::ecs::CommandBuffer;
using engine::ecs::Entity;
using engine::ecs::Phase;
using engine::ecs::Registry;
using engine::ecs::Scheduler;

namespace bench {

namespace {

struct Position {
    float x;
    float y;
};

struct Velocity {
    float x;
    float y;
};

constexpr float STEP = 0.016F;

Registry makeWorld(std::size_t entities)
{
    Registry registry;

    registry.registerComponent<Position>();
    registry.registerComponent<Velocity>();
    for (std::size_t i = 0; i < entities; ++i) {
        const Entity entity = registry.spawnEntity();
        registry.addComponent(entity, Position{static_cast<float>(i), 0.0F});
        registry.addComponent(entity, Velocity{1.0F, 0.5F});
    }
    return registry;
}

double sumPositions(Registry &registry)
{
    double sum = 0.0;

    for (const auto &position : registry.getComponents<Position>())
        sum += position ? position->x : 0.0;
    return sum;
}

void integrate(Registry &registry)
{
    auto &positions = registry.getComponents<Position>();
    auto &velocities = registry.getComponents<Velocity>();

    for (auto [position, velocity] :
         engine::ecs::makeZipper(positions, velocities)) {
        position.x += velocity.x * STEP;
        position.y += velocity.y * STEP;
    }
}

std::vector<Entity> spawnAll(Registry &registry, std::size_t entities)
{
    std::vector<Entity> spawned;

    spawned.reserve(entities);
    for (std::size_t i = 0; i < entities; ++i)
        spawned.push_back(registry.spawnEntity());
    return spawned;
}

} // namespace

std::vector<Result> runZipper(const Config &config)
{
    Registry registry = makeWorld(config.entities);
    Series series;

    for (std::size_t i = 0; i < config.repetitions; ++i)
        series.record(timed([&registry] { integrate(registry); }));
    return {series.result("zipper", "iterate Position+Velocity and update",
                          config.entities, sumPositions(registry))};
}

std::vector<Result> runSpawnKill(const Config &config)
{
    Series spawn;
    Series kill;

    for (std::size_t i = 0; i < config.repetitions; ++i) {
        Registry registry;
        std::vector<Entity> spawned;
        spawn.record(
            timed([&] { spawned = spawnAll(registry, config.entities); }));
        kill.record(timed([&] {
            for (const Entity entity : spawned)
                registry.killEntity(entity);
        }));
    }
    return {
        spawn.result("spawn", "spawnEntity, no component", config.entities,
                     0.0),
        kill.result("kill", "killEntity, no component", config.entities, 0.0)};
}

std::vector<Result> runCommandBuffer(const Config &config)
{
    Series record;
    Series flush;
    double checksum = 0.0;

    for (std::size_t i = 0; i < config.repetitions; ++i) {
        Registry registry;
        registry.registerComponent<Position>();
        const std::vector<Entity> spawned = spawnAll(registry, config.entities);
        CommandBuffer commands;
        record.record(timed([&] {
            for (const Entity entity : spawned)
                commands.add(entity, Position{1.0F, 2.0F});
        }));
        flush.record(timed([&] { commands.flush(registry); }));
        checksum = sumPositions(registry);
    }
    return {record.result("cmd-record", "record one add per entity",
                          config.entities, checksum),
            flush.result("cmd-flush", "flush those add commands",
                         config.entities, checksum)};
}

std::vector<Result> runScheduler(const Config &config)
{
    Registry registry = makeWorld(config.entities);
    Scheduler scheduler;
    Series series;

    scheduler.addSystem<Position, const Velocity>(
        "move", Phase::UPDATE,
        [](Entity, Position &position, const Velocity &velocity,
           CommandBuffer &, double step) {
            position.x += velocity.x * static_cast<float>(step);
            position.y += velocity.y * static_cast<float>(step);
        });
    scheduler.addSystem<Position>(
        "wrap", Phase::POST_UPDATE,
        [](Entity, Position &position, CommandBuffer &, double) {
            if (position.x > 1.0e9F)
                position.x = 0.0F;
        });
    scheduler.addSystem<const Position>(
        "probe", Phase::PRE_UPDATE,
        [](Entity, const Position &, CommandBuffer &, double) {});
    for (std::size_t i = 0; i < config.repetitions; ++i)
        series.record(timed([&] { scheduler.run(registry, STEP); }));
    return {series.result("scheduler", "run 3 systems over 3 phases",
                          config.entities, sumPositions(registry))};
}

} // namespace bench
