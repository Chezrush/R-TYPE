/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ChatOverlay.hpp
*/

#ifndef CHATOVERLAY_HPP_
#define CHATOVERLAY_HPP_

#include <optional>

#include "Client/Lobby/ChatService.hpp"
#include "Engine/Ecs/Systems/UIInteractionSystem.hpp"
#include "Engine/Ecs/UIOverlay.hpp"

namespace client::scenes {

/**
 * @brief In-game chat, opened with T above the game, which keeps running.
 *
 * Enter sends and closes, Escape closes. While it is open, TextEntryStarted
 * tells the input code that typing must not move the ship; TextEntryEnded
 * follows when it closes. The chat service must outlive it.
 */
class ChatOverlay : public engine::ecs::UIOverlay {
public:
    ChatOverlay(engine::ecs::UISceneContext context, lobby::ChatService &chat);

    void onExit() override;
    void onPause() override;

    [[nodiscard]] std::optional<engine::ecs::Entity>
    inputField() const noexcept;
    [[nodiscard]] std::optional<engine::ecs::Entity> logList() const noexcept;

protected:
    void activate() override;

private:
    void buildWidgets();
    void refreshLog();
    void leave();

    lobby::ChatService &_chat;
    std::optional<engine::ecs::Entity> _input;
    std::optional<engine::ecs::Entity> _log;
};

} // namespace client::scenes

#endif /* !CHATOVERLAY_HPP_ */
