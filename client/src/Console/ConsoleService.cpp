/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ConsoleService.cpp
*/

#include "Client/Console/ConsoleService.hpp"

#include "Client/Events/SettingsEvents.hpp"
#include "Client/Scenes/SceneRoute.hpp"

namespace client::console {

ConsoleService::ConsoleService(engine::core::EventBus &bus)
    : _bus(bus)
{
    _listeners.push_back(_bus.subscribe<events::ConsoleOutput>(
        [this](const events::ConsoleOutput &event) {
            _model.addOutput(event.text, event.level);
        }));
    _listeners.push_back(_bus.subscribe<events::ConsoleCommandsListed>(
        [this](const events::ConsoleCommandsListed &event) {
            _model.setCommands(event.names);
        }));
    _listeners.push_back(_bus.subscribe<events::RawKeyPressed>(
        [this](const events::RawKeyPressed &event) {
            if (event.key == settings::KEY_GRAVE && !_open)
                _bus.publish(
                    scenes::OverlayRequested{scenes::SceneRoute::CONSOLE});
        }));
}

ConsoleService::~ConsoleService()
{
    for (const auto listener : _listeners)
        _bus.unsubscribe(listener);
}

ConsoleModel &ConsoleService::model() noexcept
{
    return _model;
}

bool ConsoleService::isOpen() const noexcept
{
    return _open;
}

void ConsoleService::setOpen(bool open) noexcept
{
    _open = open;
}

} // namespace client::console
