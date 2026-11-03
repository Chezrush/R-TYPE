/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ChatService.cpp
*/

#include "Client/Lobby/ChatService.hpp"

#include <chrono>
#include <utility>

#include "Client/Events/SettingsEvents.hpp"
#include "Client/Scenes/SceneRoute.hpp"

namespace client::lobby {

namespace {

ChatService::Clock steadyClock()
{
    const auto start = std::chrono::steady_clock::now();

    return [start] {
        return std::chrono::duration<double>(std::chrono::steady_clock::now() -
                                             start)
            .count();
    };
}

} // namespace

ChatService::ChatService(engine::core::EventBus &bus,
                         const engine::core::SceneManager &scenes, Clock clock)
    : _bus(bus),
      _scenes(scenes),
      _clock(clock ? std::move(clock) : steadyClock())
{
    _listeners.push_back(_bus.subscribe<events::ChatMessageReceived>(
        [this](const events::ChatMessageReceived &message) {
            _model.add(message, _clock());
        }));
    _listeners.push_back(_bus.subscribe<events::RawKeyPressed>(
        [this](const events::RawKeyPressed &event) { onKey(event.key); }));
}

ChatService::~ChatService()
{
    for (const auto listener : _listeners)
        _bus.unsubscribe(listener);
}

void ChatService::send(const std::string &line)
{
    const ChatCommand command = ChatModel::parse(line);

    switch (command.kind) {
        case ChatCommand::Kind::SEND:
            _bus.publish(events::ChatMessageSent{command.text});
            break;
        case ChatCommand::Kind::MUTE:
        case ChatCommand::Kind::UNMUTE:
            applyMute(command);
            break;
        case ChatCommand::Kind::INVALID:
            _model.addNotice(command.text, _clock());
            break;
        default:
            break;
    }
}

void ChatService::applyMute(const ChatCommand &command)
{
    const bool mute = command.kind == ChatCommand::Kind::MUTE;

    _model.setMuted(command.player, mute);
    _model.addNotice(std::string(mute ? "Muted P" : "Unmuted P") +
                         std::to_string(command.player + 1),
                     _clock());
}

void ChatService::setGameRunning(bool running) noexcept
{
    _gameRunning = running;
}

const ChatModel &ChatService::model() const noexcept
{
    return _model;
}

ChatModel &ChatService::model() noexcept
{
    return _model;
}

void ChatService::onKey(std::uint16_t key)
{
    if (key != settings::KEY_T || !_gameRunning || _scenes.empty() ||
        _scenes.currentScene().isOverlay())
        return;
    _bus.publish(scenes::OverlayRequested{scenes::SceneRoute::CHAT});
}

} // namespace client::lobby
