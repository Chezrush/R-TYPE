/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** IScene.hpp
*/

#ifndef ISCENE_HPP_
#define ISCENE_HPP_

namespace engine::core {

/**
 * @brief Interface representing an application state or screen.
 *
 * Scenes can represent game states (e.g. Main Menu, Lobby, Gameplay)
 * or modal overlays (e.g. Pause Menu, Game Over).
 */
class IScene {
public:
    IScene() = default;
    virtual ~IScene() = default;

    IScene(const IScene &) = delete;
    IScene &operator=(const IScene &) = delete;
    IScene(IScene &&) = delete;
    IScene &operator=(IScene &&) = delete;

    /** @brief Called when the scene is loaded or brought to the top. */
    virtual void onEnter() = 0;

    /** @brief Called when the scene is unloaded or removed. */
    virtual void onExit() = 0;

    /** @brief Called when another scene is pushed on top of this scene. */
    virtual void onPause() = 0;

    /** @brief Called when the scene on top is popped and this resumes. */
    virtual void onResume() = 0;

    /** @brief Updates scene logic with delta time in seconds. */
    virtual void update(float dt) = 0;

    /** @brief Renders the scene. */
    virtual void render() = 0;

    /** @brief Handles user input events. */
    virtual void handleInput() = 0;

    /**
     * @brief Whether this scene is a transparent overlay.
     *
     * When true, the scene below this one in the stack will still be rendered
     * before this scene is drawn.
     */
    [[nodiscard]] virtual bool isOverlay() const noexcept = 0;

    /**
     * @brief Whether the scene below keeps updating under this overlay.
     *
     * For a game that goes on behind a menu. The input stays for the top scene.
     */
    [[nodiscard]] virtual bool updatesSceneBelow() const noexcept
    {
        return false;
    }
};

} // namespace engine::core

#endif /* !ISCENE_HPP_ */
