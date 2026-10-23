/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** CommandBuffer.hpp
*/

#ifndef COMMANDBUFFER_HPP_
#define COMMANDBUFFER_HPP_

#include <cstddef>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "Engine/Ecs/Entity.hpp"

namespace engine::ecs {

class Registry;

/** @brief One recorded structural change, applied later to a registry. */
class ICommand {
public:
    ICommand() = default;
    virtual ~ICommand() = default;

    ICommand(const ICommand &) = delete;
    ICommand &operator=(const ICommand &) = delete;
    ICommand(ICommand &&) = delete;
    ICommand &operator=(ICommand &&) = delete;

    virtual void apply(Registry &registry) = 0;
};

namespace detail {

/** @brief Command adding a component, owning it until it is applied. */
template <typename Component>
class AddCommand final : public ICommand {
public:
    AddCommand(Entity entity, Component component);

    void apply(Registry &registry) override;

private:
    Entity _entity;
    Component _component;
};

/** @brief Command removing a component of one type from an entity. */
template <typename Component>
class RemoveCommand final : public ICommand {
public:
    explicit RemoveCommand(Entity entity);

    void apply(Registry &registry) override;

private:
    Entity _entity;
};

} // namespace detail

/**
 * @brief Records structural changes of a registry to apply them later.
 *
 * Spawning or killing an entity while a system iterates over the component
 * arrays would invalidate the iteration. The buffer keeps the changes aside
 * and never touches a registry while recording. Commands own their
 * components, so move-only components are supported. A buffer is not
 * thread-safe: each system or thread records in its own.
 */
class CommandBuffer {
public:
    using SpawnCallback = std::function<void(Entity)>;

    /**
     * @brief Records the creation of an entity.
     *
     * @param onSpawn Called with the new handle once the entity exists, the
     * only way to learn it since nothing is created while recording.
     */
    void spawn(SpawnCallback onSpawn = {});

    /** @brief Records the destruction of an entity and its components. */
    void kill(Entity entity);

    /** @brief Records the addition of a component to an entity. */
    template <typename Component>
    void add(Entity entity, Component component);

    /**
     * @brief Records the addition of a component built from arguments.
     *
     * The component is built immediately: the arguments are evaluated at
     * recording time, not when the command is applied.
     */
    template <typename Component, typename... Args>
    void emplace(Entity entity, Args &&...args);

    /** @brief Records the removal of a component from an entity. */
    template <typename Component>
    void remove(Entity entity);

    /** @return The number of recorded commands. */
    [[nodiscard]] std::size_t size() const noexcept;

    [[nodiscard]] bool empty() const noexcept;

    /** @brief Drops every recorded command; owned components are destroyed. */
    void clear() noexcept;

private:
    void push(std::unique_ptr<ICommand> command);

    std::vector<std::unique_ptr<ICommand>> _commands;
};

} // namespace engine::ecs

#include "Engine/Ecs/CommandBuffer.tpp"

#endif /* !COMMANDBUFFER_HPP_ */
