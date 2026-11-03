/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ChatService.hpp
*/

#ifndef CHATSERVICE_HPP_
#define CHATSERVICE_HPP_

#include <functional>
#include <string>
#include <vector>

#include "Client/Lobby/ChatModel.hpp"
#include "Engine/Core/EventBus.hpp"
#include "Engine/Core/SceneManager.hpp"

namespace client::lobby {

/**
 * @brief Keeps the chat for the lobby and the game, and opens it in game.
 *
 * It records ChatMessageReceived at any time. send() handles the local
 * commands and publishes ChatMessageSent for the rest; the server echoes our
 * own messages back. While enabled (by the game scene), T (RawKeyPressed with
 * KEY_T) asks for the CHAT overlay if the game itself is on top. The bus and
 * the scene manager must outlive this object.
 */
class ChatService {
public:
    using Clock = std::function<double()>;

    ChatService(engine::core::EventBus &bus,
                const engine::core::SceneManager &scenes, Clock clock = {});
    ~ChatService();

    ChatService(const ChatService &) = delete;
    ChatService &operator=(const ChatService &) = delete;
    ChatService(ChatService &&) = delete;
    ChatService &operator=(ChatService &&) = delete;

    /** @brief Sends a typed line, or applies a /mute command. */
    void send(const std::string &line);

    /** @brief Lets T open the chat, while a game is running. */
    void setGameRunning(bool running) noexcept;

    [[nodiscard]] const ChatModel &model() const noexcept;
    [[nodiscard]] ChatModel &model() noexcept;

private:
    void onKey(std::uint16_t key);
    void applyMute(const ChatCommand &command);

    engine::core::EventBus &_bus;
    const engine::core::SceneManager &_scenes;
    Clock _clock;
    std::vector<engine::core::EventBus::ListenerId> _listeners;
    ChatModel _model;
    bool _gameRunning = false;
};

} // namespace client::lobby

#endif /* !CHATSERVICE_HPP_ */
