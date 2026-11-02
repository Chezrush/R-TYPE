/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** SceneManager.hpp
*/

#ifndef SCENEMANAGER_HPP_
#define SCENEMANAGER_HPP_

#include <cstddef>
#include <memory>
#include <vector>

#include "Engine/Core/IScene.hpp"

namespace engine::core {

/**
 * @brief Stack-based scene and state machine manager.
 *
 * Owns a stack of scenes. Supports pushing modal overlays, popping scenes,
 * and switching base scenes while invoking lifecycle callbacks.
 */
class SceneManager {
public:
    SceneManager() = default;
    ~SceneManager();

    SceneManager(const SceneManager &) = delete;
    SceneManager &operator=(const SceneManager &) = delete;
    SceneManager(SceneManager &&) noexcept = default;
    SceneManager &operator=(SceneManager &&) noexcept = default;

    /**
     * @brief Pushes a new scene on top of the stack.
     *
     * Calls onPause() on the previous top scene (if any) and onEnter()
     * on the new scene.
     */
    void pushScene(std::unique_ptr<IScene> scene);

    /**
     * @brief Pops the top scene from the stack.
     *
     * Calls onExit() on the popped scene and onResume() on the new top
     * scene (if any).
     */
    void popScene();

    /**
     * @brief Clears the stack and pushes a new base scene.
     */
    void switchScene(std::unique_ptr<IScene> scene);

    /** @brief Clears all scenes, calling onExit() on each. */
    void clear() noexcept;

    /**
     * @brief Updates the active top scene.
     *
     * The scenes below it are updated first, in order, as long as the scene
     * above them says so (see IScene::updatesSceneBelow()).
     */
    void update(float dt);

    /**
     * @brief Renders the active scenes.
     *
     * If the top scene is an overlay, scenes below it are rendered in order.
     */
    void render();

    /** @brief Passes input handling to the active top scene. */
    void handleInput();

    /** @brief Whether the scene stack is empty. */
    [[nodiscard]] bool empty() const noexcept;

    /** @brief Number of active scenes in the stack. */
    [[nodiscard]] std::size_t size() const noexcept;

    /** @brief Returns a reference to the active top scene. */
    [[nodiscard]] IScene &currentScene();

    /** @brief Returns a const reference to the active top scene. */
    [[nodiscard]] const IScene &currentScene() const;

private:
    std::vector<std::unique_ptr<IScene>> _scenes;
};

} // namespace engine::core

#endif /* !SCENEMANAGER_HPP_ */
