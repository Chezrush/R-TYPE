/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** CommandBuffer.cpp
*/

#include "Engine/Ecs/CommandBuffer.hpp"

#include "Engine/Ecs/Registry.hpp"

namespace engine::ecs {

namespace {

class SpawnCommand final : public ICommand {
public:
    explicit SpawnCommand(CommandBuffer::SpawnCallback onSpawn)
        : _onSpawn(std::move(onSpawn))
    {}

    void apply(Registry &registry) override
    {
        const Entity entity = registry.spawnEntity();

        if (_onSpawn)
            _onSpawn(entity);
    }

private:
    CommandBuffer::SpawnCallback _onSpawn;
};

class KillCommand final : public ICommand {
public:
    explicit KillCommand(Entity entity)
        : _entity(entity)
    {}

    void apply(Registry &registry) override
    {
        registry.killEntity(_entity);
    }

private:
    Entity _entity;
};

} // namespace

void CommandBuffer::spawn(SpawnCallback onSpawn)
{
    push(std::make_unique<SpawnCommand>(std::move(onSpawn)));
}

void CommandBuffer::kill(Entity entity)
{
    push(std::make_unique<KillCommand>(entity));
}

std::size_t CommandBuffer::size() const noexcept
{
    return _commands.size();
}

bool CommandBuffer::empty() const noexcept
{
    return _commands.empty();
}

void CommandBuffer::clear() noexcept
{
    _commands.clear();
}

void CommandBuffer::push(std::unique_ptr<ICommand> command)
{
    _commands.push_back(std::move(command));
}

} // namespace engine::ecs
