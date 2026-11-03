/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ConsoleOverlay.hpp
*/

#ifndef CONSOLEOVERLAY_HPP_
#define CONSOLEOVERLAY_HPP_

#include <optional>

#include "Client/Console/ConsoleService.hpp"
#include "Client/Scenes/SceneRoute.hpp"
#include "Engine/Ecs/Systems/UIInteractionSystem.hpp"
#include "Engine/Ecs/UIOverlay.hpp"

namespace client::scenes {

/**
 * @brief Developer console above the game, which keeps running.
 *
 * Enter publishes ConsoleCommandSubmitted, Up / Down browse the history, Tab
 * completes the command name, Escape or the key above Tab close it. While it is
 * open, TextEntryStarted tells the input code that typing must not move the
 * ship; TextEntryEnded follows when it closes. The service must outlive it.
 */
class ConsoleOverlay : public engine::ecs::UIOverlay {
public:
    ConsoleOverlay(engine::ecs::UISceneContext context,
                   console::ConsoleService &service);

    void onExit() override;
    void onPause() override;
    void handleInput() override;

    [[nodiscard]] std::optional<engine::ecs::Entity>
    inputField() const noexcept;
    [[nodiscard]] std::optional<engine::ecs::Entity> logList() const noexcept;

protected:
    void activate() override;

private:
    void buildWidgets();
    void listenToWidgets();
    void onKey(engine::ecs::UIKey key);
    void submit(const std::string &line);
    void complete();
    void setLine(const std::string &line);
    void refreshLog();
    void close();
    void leave();

    console::ConsoleService &_service;
    std::optional<engine::ecs::Entity> _input;
    std::optional<engine::ecs::Entity> _log;
    bool _discardInput = false;
};

/** @brief Gives the router the console. */
void addConsoleRoute(SceneRouter &router,
                     const engine::ecs::UISceneContext &context,
                     console::ConsoleService &service);

} // namespace client::scenes

#endif /* !CONSOLEOVERLAY_HPP_ */
