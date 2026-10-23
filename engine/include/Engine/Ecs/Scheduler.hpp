/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Scheduler.hpp
*/

#ifndef SCHEDULER_HPP_
#define SCHEDULER_HPP_

#include <array>
#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include "Engine/Ecs/Access.hpp"
#include "Engine/Ecs/Phase.hpp"

namespace engine::ecs {

class Registry;

/**
 * @brief Runs systems phase by phase, in a deterministic order.
 *
 * Phases run in the order of the Phase enumeration. Inside a phase, systems
 * run in registration order: there is no dependency graph, the declared
 * Access is kept for conflict detection. A system that throws stops the
 * frame and the exception reaches the caller of run().
 */
class Scheduler {
public:
    using SystemId = std::size_t;
    using SystemFunction = std::function<void(Registry &, double)>;

    /**
     * @brief Registers an enabled system at the end of a phase.
     *
     * @param name Label of the system, for diagnostics.
     * @param phase Phase in which the system runs.
     * @param access Components the system declares to read and write.
     * @param function Called each frame with the registry and the time step
     * in seconds.
     * @return An identifier, never reused by this scheduler.
     * @throws std::invalid_argument if the function is empty.
     * @throws std::logic_error if called while the scheduler is running.
     */
    SystemId addSystem(std::string name, Phase phase, Access access,
                       SystemFunction function);

    /**
     * @brief Removes a system, the order of the others is unchanged.
     *
     * @return false if no such system is registered.
     * @throws std::logic_error if called while the scheduler is running.
     */
    bool removeSystem(SystemId id);

    /**
     * @brief Enables or disables a system without unregistering it.
     *
     * Safe to call from a running system: a system disabled before its turn
     * is skipped in the current frame.
     * @return false if no such system is registered.
     */
    bool setEnabled(SystemId id, bool enabled) noexcept;

    /** @return true if the system exists and is enabled. */
    [[nodiscard]] bool isEnabled(SystemId id) const noexcept;

    /** @brief Number of registered systems, enabled or not. */
    [[nodiscard]] std::size_t size() const noexcept;

    /**
     * @brief Runs every enabled system once, phase after phase.
     *
     * @param deltaTime Time step in seconds given to each system.
     * @throws std::logic_error if the scheduler is already running.
     */
    void run(Registry &registry, double deltaTime);

private:
    struct System {
        SystemId id;
        std::string name;
        Access access;
        SystemFunction function;
        bool enabled;
    };

    using Group = std::vector<System>;

    [[nodiscard]] System *find(SystemId id) noexcept;
    [[nodiscard]] const System *find(SystemId id) const noexcept;
    void ensureIdle() const;
    static void runGroup(const Group &group, Registry &registry,
                         double deltaTime);

    std::array<Group, PHASE_COUNT> _phases;
    SystemId _nextId = 0;
    bool _running = false;
};

} // namespace engine::ecs

#endif /* !SCHEDULER_HPP_ */
