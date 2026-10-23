/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Scheduler.cpp
*/

#include "Engine/Ecs/Scheduler.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include "Engine/Ecs/IndexedZipper.hpp"
#include "Engine/Ecs/Registry.hpp"

using engine::ecs::Access;
using engine::ecs::CommandBuffer;
using engine::ecs::Entity;
using engine::ecs::Phase;
using engine::ecs::Registry;
using engine::ecs::Scheduler;

namespace {

using Log = std::vector<std::string>;

Scheduler::SystemFunction logTo(Log &log, const std::string &label)
{
    return [&log, label](Registry &, CommandBuffer &, double) {
        log.push_back(label);
    };
}

} // namespace

TEST_CASE("an empty scheduler runs without effect", "[ecs][scheduler]")
{
    Registry registry;
    Scheduler scheduler;

    scheduler.run(registry, 0.016);

    REQUIRE(scheduler.size() == 0);
}

TEST_CASE("a system receives the registry and the time step",
          "[ecs][scheduler]")
{
    Registry registry;
    Scheduler scheduler;
    const Registry *seenRegistry = nullptr;
    double seenStep = 0.0;

    scheduler.addSystem("probe", Phase::UPDATE, Access(),
                        [&](Registry &given, CommandBuffer &, double step) {
                            seenRegistry = &given;
                            seenStep = step;
                        });
    scheduler.run(registry, 0.25);

    REQUIRE(seenRegistry == &registry);
    REQUIRE(seenStep == 0.25);
}

TEST_CASE("a system can change the registry it receives", "[ecs][scheduler]")
{
    Registry registry;
    Scheduler scheduler;

    scheduler.addSystem("spawner", Phase::UPDATE, Access(),
                        [](Registry &given, CommandBuffer &, double) {
                            static_cast<void>(given.spawnEntity());
                        });
    scheduler.run(registry, 0.0);
    scheduler.run(registry, 0.0);

    REQUIRE(registry.aliveCount() == 2);
}

TEST_CASE("phases run in their fixed order whatever the registration order",
          "[ecs][scheduler]")
{
    Registry registry;
    Scheduler scheduler;
    Log log;

    scheduler.addSystem("post", Phase::POST_UPDATE, Access(),
                        logTo(log, "post"));
    scheduler.addSystem("update", Phase::UPDATE, Access(),
                        logTo(log, "update"));
    scheduler.addSystem("pre", Phase::PRE_UPDATE, Access(), logTo(log, "pre"));
    scheduler.run(registry, 0.0);

    REQUIRE(log == Log{"pre", "update", "post"});
}

TEST_CASE("systems of a phase run in registration order", "[ecs][scheduler]")
{
    Registry registry;
    Scheduler scheduler;
    Log log;

    scheduler.addSystem("a", Phase::UPDATE, Access(), logTo(log, "a"));
    scheduler.addSystem("b", Phase::UPDATE, Access(), logTo(log, "b"));
    scheduler.addSystem("c", Phase::UPDATE, Access(), logTo(log, "c"));
    scheduler.run(registry, 0.0);
    scheduler.run(registry, 0.0);

    REQUIRE(log == Log{"a", "b", "c", "a", "b", "c"});
}

TEST_CASE("a new system starts enabled", "[ecs][scheduler]")
{
    Scheduler scheduler;
    const auto id =
        scheduler.addSystem("a", Phase::UPDATE, Access(),
                            [](Registry &, CommandBuffer &, double) {});

    REQUIRE(scheduler.isEnabled(id));
    REQUIRE(scheduler.size() == 1);
}

TEST_CASE("a disabled system is skipped and can be enabled again",
          "[ecs][scheduler]")
{
    Registry registry;
    Scheduler scheduler;
    Log log;

    scheduler.addSystem("a", Phase::UPDATE, Access(), logTo(log, "a"));
    const auto b =
        scheduler.addSystem("b", Phase::UPDATE, Access(), logTo(log, "b"));

    REQUIRE(scheduler.setEnabled(b, false));
    REQUIRE_FALSE(scheduler.isEnabled(b));
    scheduler.run(registry, 0.0);
    REQUIRE(log == Log{"a"});
    REQUIRE(scheduler.setEnabled(b, true));
    scheduler.run(registry, 0.0);
    REQUIRE(log == Log{"a", "a", "b"});
}

TEST_CASE("a removed system no longer runs and the others keep their order",
          "[ecs][scheduler]")
{
    Registry registry;
    Scheduler scheduler;
    Log log;

    scheduler.addSystem("a", Phase::UPDATE, Access(), logTo(log, "a"));
    const auto b =
        scheduler.addSystem("b", Phase::UPDATE, Access(), logTo(log, "b"));
    scheduler.addSystem("c", Phase::UPDATE, Access(), logTo(log, "c"));

    REQUIRE(scheduler.removeSystem(b));
    scheduler.run(registry, 0.0);

    REQUIRE(log == Log{"a", "c"});
    REQUIRE(scheduler.size() == 2);
}

TEST_CASE("identifiers are not reused after a removal", "[ecs][scheduler]")
{
    Scheduler scheduler;
    const auto first =
        scheduler.addSystem("a", Phase::UPDATE, Access(),
                            [](Registry &, CommandBuffer &, double) {});

    REQUIRE(scheduler.removeSystem(first));
    const auto second =
        scheduler.addSystem("b", Phase::UPDATE, Access(),
                            [](Registry &, CommandBuffer &, double) {});

    REQUIRE(second != first);
    REQUIRE_FALSE(scheduler.isEnabled(first));
}

TEST_CASE("an unknown system is reported by the return value",
          "[ecs][scheduler]")
{
    Scheduler scheduler;

    REQUIRE_FALSE(scheduler.removeSystem(42));
    REQUIRE_FALSE(scheduler.setEnabled(42, true));
    REQUIRE_FALSE(scheduler.isEnabled(42));
}

TEST_CASE("a system without function is rejected", "[ecs][scheduler]")
{
    Scheduler scheduler;

    REQUIRE_THROWS_AS(scheduler.addSystem("empty", Phase::UPDATE, Access(), {}),
                      std::invalid_argument);
    REQUIRE(scheduler.size() == 0);
}

TEST_CASE("a system disabled earlier in the frame is skipped",
          "[ecs][scheduler]")
{
    Registry registry;
    Scheduler scheduler;
    Log log;
    Scheduler::SystemId victim = 0;

    scheduler.addSystem("killer", Phase::UPDATE, Access(),
                        [&](Registry &, CommandBuffer &, double) {
                            scheduler.setEnabled(victim, false);
                        });
    victim = scheduler.addSystem("victim", Phase::UPDATE, Access(),
                                 logTo(log, "victim"));
    scheduler.run(registry, 0.0);

    REQUIRE(log.empty());
}

TEST_CASE("the set of systems cannot change while running", "[ecs][scheduler]")
{
    Registry registry;
    Scheduler scheduler;
    bool addRejected = false;
    bool removeRejected = false;
    bool rerunRejected = false;

    const auto self = scheduler.addSystem(
        "meddler", Phase::UPDATE, Access(),
        [&](Registry &given, CommandBuffer &, double) {
            try {
                scheduler.addSystem("x", Phase::UPDATE, Access(),
                                    [](Registry &, CommandBuffer &, double) {});
            } catch (const std::logic_error &) {
                addRejected = true;
            }
            try {
                scheduler.removeSystem(0);
            } catch (const std::logic_error &) {
                removeRejected = true;
            }
            try {
                scheduler.run(given, 0.0);
            } catch (const std::logic_error &) {
                rerunRejected = true;
            }
        });
    scheduler.run(registry, 0.0);

    REQUIRE(scheduler.isEnabled(self));
    REQUIRE(addRejected);
    REQUIRE(removeRejected);
    REQUIRE(rerunRejected);
    REQUIRE(scheduler.size() == 1);
}

TEST_CASE("the scheduler stays usable after a system throws",
          "[ecs][scheduler]")
{
    Registry registry;
    Scheduler scheduler;
    int runs = 0;

    const auto thrower =
        scheduler.addSystem("thrower", Phase::PRE_UPDATE, Access(),
                            [](Registry &, CommandBuffer &, double) {
                                throw std::runtime_error("boom");
                            });
    scheduler.addSystem("counter", Phase::UPDATE, Access(),
                        [&](Registry &, CommandBuffer &, double) { ++runs; });

    REQUIRE_THROWS_AS(scheduler.run(registry, 0.0), std::runtime_error);
    REQUIRE(runs == 0);
    scheduler.setEnabled(thrower, false);
    scheduler.run(registry, 0.0);

    REQUIRE(runs == 1);
}

TEST_CASE("the declared access is accepted for any component types",
          "[ecs][scheduler]")
{
    Scheduler scheduler;
    Access access;

    access.read<int>().write<double>();
    scheduler.addSystem("typed", Phase::UPDATE, access,
                        [](Registry &, CommandBuffer &, double) {});

    REQUIRE(scheduler.size() == 1);
}

namespace {

struct Position {
    int x;
};

struct Velocity {
    int dx;
};

Scheduler::SystemFunction noop()
{
    return [](Registry &, CommandBuffer &, double) {};
}

} // namespace

TEST_CASE("a change recorded by a system is applied after its phase",
          "[ecs][scheduler][commands]")
{
    Registry registry;
    Scheduler scheduler;
    std::size_t seenBefore = 99;
    std::size_t seenLater = 99;

    scheduler.addSystem(
        "spawner", Phase::PRE_UPDATE, Access(),
        [](Registry &, CommandBuffer &commands, double) { commands.spawn(); });
    scheduler.addSystem("same-phase", Phase::PRE_UPDATE, Access(),
                        [&](Registry &given, CommandBuffer &, double) {
                            seenBefore = given.aliveCount();
                        });
    scheduler.addSystem("next-phase", Phase::UPDATE, Access(),
                        [&](Registry &given, CommandBuffer &, double) {
                            seenLater = given.aliveCount();
                        });
    scheduler.run(registry, 0.0);

    REQUIRE(seenBefore == 0);
    REQUIRE(seenLater == 1);
}

TEST_CASE("the last phase is flushed before run returns",
          "[ecs][scheduler][commands]")
{
    Registry registry;
    Scheduler scheduler;

    scheduler.addSystem(
        "spawner", Phase::POST_UPDATE, Access(),
        [](Registry &, CommandBuffer &commands, double) { commands.spawn(); });
    scheduler.run(registry, 0.0);
    scheduler.run(registry, 0.0);

    REQUIRE(registry.aliveCount() == 2);
}

TEST_CASE("a kill recorded in a phase is applied before the next phase",
          "[ecs][scheduler][commands]")
{
    Registry registry;
    Scheduler scheduler;
    const Entity victim = registry.spawnEntity();
    bool aliveInNextPhase = true;

    scheduler.addSystem("killer", Phase::PRE_UPDATE, Access(),
                        [victim](Registry &, CommandBuffer &commands, double) {
                            commands.kill(victim);
                        });
    scheduler.addSystem("observer", Phase::UPDATE, Access(),
                        [&](Registry &given, CommandBuffer &, double) {
                            aliveInNextPhase = given.isAlive(victim);
                        });
    scheduler.run(registry, 0.0);

    REQUIRE_FALSE(aliveInNextPhase);
}

TEST_CASE("a system can kill entities while iterating over them",
          "[ecs][scheduler][commands]")
{
    Registry registry;
    Scheduler scheduler;
    std::vector<Entity> entities;

    registry.registerComponent<Position>();
    for (int i = 0; i < 4; ++i) {
        entities.push_back(registry.spawnEntity());
        registry.addComponent(entities.back(), Position{i});
    }
    scheduler.addSystem("reaper", Phase::UPDATE, Access().write<Position>(),
                        [&](Registry &given, CommandBuffer &commands, double) {
                            for (auto [index, position] :
                                 engine::ecs::makeIndexedZipper(
                                     given.getComponents<Position>()))
                                if (position.x % 2 == 0)
                                    commands.kill(entities[index]);
                        });
    scheduler.run(registry, 0.0);

    REQUIRE(registry.aliveCount() == 2);
}

TEST_CASE("the commands of an aborted frame are discarded",
          "[ecs][scheduler][commands]")
{
    Registry registry;
    Scheduler scheduler;

    scheduler.addSystem(
        "spawner", Phase::UPDATE, Access(),
        [](Registry &, CommandBuffer &commands, double) { commands.spawn(); });
    const auto thrower =
        scheduler.addSystem("thrower", Phase::UPDATE, Access(),
                            [](Registry &, CommandBuffer &, double) {
                                throw std::runtime_error("boom");
                            });

    REQUIRE_THROWS_AS(scheduler.run(registry, 0.0), std::runtime_error);
    scheduler.setEnabled(thrower, false);
    scheduler.run(registry, 0.0);

    REQUIRE(registry.aliveCount() == 1);
}

TEST_CASE("an unregistered component aborts the frame and is reported",
          "[ecs][scheduler][commands]")
{
    Registry registry;
    Scheduler scheduler;
    const Entity entity = registry.spawnEntity();

    scheduler.addSystem("adder", Phase::UPDATE, Access(),
                        [entity](Registry &, CommandBuffer &commands, double) {
                            commands.add<Velocity>(entity, Velocity{1});
                        });

    REQUIRE_THROWS_AS(scheduler.run(registry, 0.0), std::logic_error);
}

TEST_CASE("no conflict is reported without declared accesses",
          "[ecs][scheduler][conflicts]")
{
    Scheduler scheduler;

    scheduler.addSystem("a", Phase::UPDATE, Access(), noop());
    scheduler.addSystem("b", Phase::UPDATE, Access(), noop());

    REQUIRE(scheduler.findConflicts().empty());
}

TEST_CASE("two systems writing one component conflict in a phase",
          "[ecs][scheduler][conflicts]")
{
    Scheduler scheduler;

    const auto first = scheduler.addSystem("move", Phase::UPDATE,
                                           Access().write<Position>(), noop());
    const auto second = scheduler.addSystem("wrap", Phase::UPDATE,
                                            Access().write<Position>(), noop());
    const auto conflicts = scheduler.findConflicts();

    REQUIRE(conflicts.size() == 1);
    REQUIRE(conflicts[0].phase == Phase::UPDATE);
    REQUIRE(conflicts[0].first == first);
    REQUIRE(conflicts[0].second == second);
    REQUIRE(conflicts[0].firstName == "move");
    REQUIRE(conflicts[0].secondName == "wrap");
}

TEST_CASE("a read and a write of one component conflict",
          "[ecs][scheduler][conflicts]")
{
    Scheduler scheduler;

    scheduler.addSystem("read", Phase::UPDATE, Access().read<Position>(),
                        noop());
    scheduler.addSystem("write", Phase::UPDATE, Access().write<Position>(),
                        noop());

    REQUIRE(scheduler.findConflicts().size() == 1);
}

TEST_CASE("reads and disjoint writes do not conflict",
          "[ecs][scheduler][conflicts]")
{
    Scheduler scheduler;

    scheduler.addSystem("a", Phase::UPDATE, Access().read<Position>(), noop());
    scheduler.addSystem("b", Phase::UPDATE, Access().read<Position>(), noop());
    scheduler.addSystem("c", Phase::UPDATE, Access().write<Velocity>(), noop());

    REQUIRE(scheduler.findConflicts().empty());
}

TEST_CASE("systems of different phases never conflict",
          "[ecs][scheduler][conflicts]")
{
    Scheduler scheduler;

    scheduler.addSystem("a", Phase::PRE_UPDATE, Access().write<Position>(),
                        noop());
    scheduler.addSystem("b", Phase::UPDATE, Access().write<Position>(), noop());

    REQUIRE(scheduler.findConflicts().empty());
}

TEST_CASE("disabled and removed systems are left out of the conflicts",
          "[ecs][scheduler][conflicts]")
{
    Scheduler scheduler;

    scheduler.addSystem("a", Phase::UPDATE, Access().write<Position>(), noop());
    const auto b = scheduler.addSystem("b", Phase::UPDATE,
                                       Access().write<Position>(), noop());
    const auto c = scheduler.addSystem("c", Phase::UPDATE,
                                       Access().write<Position>(), noop());

    REQUIRE(scheduler.findConflicts().size() == 3);
    scheduler.setEnabled(b, false);
    REQUIRE(scheduler.findConflicts().size() == 1);
    scheduler.removeSystem(c);
    REQUIRE(scheduler.findConflicts().empty());
}

TEST_CASE("conflicts are listed by phase then registration order",
          "[ecs][scheduler][conflicts]")
{
    Scheduler scheduler;

    scheduler.addSystem("u1", Phase::UPDATE, Access().write<Position>(),
                        noop());
    scheduler.addSystem("u2", Phase::UPDATE, Access().write<Position>(),
                        noop());
    scheduler.addSystem("p1", Phase::PRE_UPDATE, Access().write<Velocity>(),
                        noop());
    scheduler.addSystem("p2", Phase::PRE_UPDATE, Access().write<Velocity>(),
                        noop());
    const auto conflicts = scheduler.findConflicts();

    REQUIRE(conflicts.size() == 2);
    REQUIRE(conflicts[0].phase == Phase::PRE_UPDATE);
    REQUIRE(conflicts[1].phase == Phase::UPDATE);
}
