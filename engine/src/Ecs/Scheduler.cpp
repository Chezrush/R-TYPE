/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Scheduler.cpp
*/

#include "Engine/Ecs/Scheduler.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

namespace engine::ecs {

namespace {

class RunningGuard {
public:
    explicit RunningGuard(bool &flag)
        : _flag(flag)
    {
        if (_flag)
            throw std::logic_error("the scheduler is already running");
        _flag = true;
    }

    ~RunningGuard()
    {
        _flag = false;
    }

    RunningGuard(const RunningGuard &) = delete;
    RunningGuard &operator=(const RunningGuard &) = delete;
    RunningGuard(RunningGuard &&) = delete;
    RunningGuard &operator=(RunningGuard &&) = delete;

private:
    bool &_flag;
};

} // namespace

Scheduler::SystemId Scheduler::addSystem(std::string name, Phase phase,
                                         Access access, SystemFunction function)
{
    const auto index = static_cast<std::size_t>(phase);

    ensureIdle();
    if (!function || index >= PHASE_COUNT)
        throw std::invalid_argument("invalid system");
    _phases[index].push_back(System{_nextId, std::move(name), std::move(access),
                                    std::move(function), true});
    return _nextId++;
}

bool Scheduler::removeSystem(SystemId id)
{
    ensureIdle();
    for (Group &group : _phases) {
        const auto removed = std::erase_if(
            group, [id](const System &system) { return system.id == id; });
        if (removed > 0)
            return true;
    }
    return false;
}

bool Scheduler::setEnabled(SystemId id, bool enabled) noexcept
{
    System *system = find(id);

    if (system == nullptr)
        return false;
    system->enabled = enabled;
    return true;
}

bool Scheduler::isEnabled(SystemId id) const noexcept
{
    const System *system = find(id);

    return system != nullptr && system->enabled;
}

std::size_t Scheduler::size() const noexcept
{
    std::size_t count = 0;

    for (const Group &group : _phases)
        count += group.size();
    return count;
}

void Scheduler::run(Registry &registry, double deltaTime)
{
    const RunningGuard guard(_running);

    for (const Group &group : _phases)
        runGroup(group, registry, deltaTime);
}

Scheduler::System *Scheduler::find(SystemId id) noexcept
{
    return const_cast<System *>(std::as_const(*this).find(id));
}

const Scheduler::System *Scheduler::find(SystemId id) const noexcept
{
    for (const Group &group : _phases) {
        const auto it = std::find_if(
            group.begin(), group.end(),
            [id](const System &system) { return system.id == id; });
        if (it != group.end())
            return &*it;
    }
    return nullptr;
}

void Scheduler::ensureIdle() const
{
    if (_running)
        throw std::logic_error("the scheduler is running");
}

void Scheduler::runGroup(const Group &group, Registry &registry,
                         double deltaTime)
{
    for (const System &system : group) {
        if (system.enabled)
            system.function(registry, deltaTime);
    }
}

} // namespace engine::ecs
