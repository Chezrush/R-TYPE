/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameInput.cpp
*/

#include "Client/Flow/GameInput.hpp"

#include <array>
#include <variant>

namespace client::flow {

namespace {

using client::net::ClientSession;
using settings::GameAction;

struct MoveKey {
    GameAction action;
    settings::ScanCode alternative;
    unsigned bit;
};

constexpr settings::ScanCode KEY_W = 0x1A;
constexpr settings::ScanCode KEY_A = 0x04;
constexpr settings::ScanCode KEY_S = 0x16;
constexpr settings::ScanCode KEY_D = 0x07;
constexpr std::uint8_t LAST_REJECT_REASON = 5;

const std::array<MoveKey, 4> MOVES = {{
    {GameAction::MOVE_UP, KEY_W, rtype::protocol::KEY_UP},
    {GameAction::MOVE_DOWN, KEY_S, rtype::protocol::KEY_DOWN},
    {GameAction::MOVE_LEFT, KEY_A, rtype::protocol::KEY_LEFT},
    {GameAction::MOVE_RIGHT, KEY_D, rtype::protocol::KEY_RIGHT},
}};

bool held(const IGameDevice &device, const settings::KeyBindings &bindings,
          const MoveKey &move)
{
    return device.isKeyDown(settings::bindingOf(bindings, move.action).key) ||
           device.isKeyDown(move.alternative);
}

} // namespace

std::uint8_t readKeys(const IGameDevice &device,
                      const settings::ClientSettings &settings)
{
    const auto &bindings = settings.bindings;
    const auto shootKey = settings::bindingOf(bindings, GameAction::SHOOT).key;
    unsigned keys = 0;

    for (const MoveKey &move : MOVES) {
        if (held(device, bindings, move)) {
            keys |= move.bit;
        }
    }
    if (bindings.autoFire || device.isKeyDown(shootKey)) {
        keys |= rtype::protocol::KEY_SHOOT;
    }
    return static_cast<std::uint8_t>(keys);
}

std::string bannerOf(ClientSession::State state)
{
    switch (state) {
        case ClientSession::State::CONNECTING:
            return "Connecting...";
        case ClientSession::State::REJECTED:
            return "Server refused the connection";
        case ClientSession::State::FAILED:
            return "Connection lost";
        case ClientSession::State::CLOSED:
            return "Disconnected";
        case ClientSession::State::CONNECTED:
            break;
    }
    return {};
}

events::ConnectionFailure failureOf(ClientSession::State state,
                                    std::uint8_t rejectReason) noexcept
{
    if (state == ClientSession::State::REJECTED && rejectReason >= 1 &&
        rejectReason <= LAST_REJECT_REASON) {
        return static_cast<events::ConnectionFailure>(rejectReason);
    }
    return events::ConnectionFailure::TIMEOUT;
}

void playSounds(const rtype::protocol::Message &message, IGameDevice &device)
{
    namespace proto = rtype::protocol;

    if (const auto *spawn = std::get_if<proto::Spawn>(&message)) {
        if (spawn->kind ==
            static_cast<std::uint8_t>(proto::EntityKind::PLAYER_MISSILE)) {
            device.playShot();
        }
    } else if (const auto *destroy = std::get_if<proto::Destroy>(&message)) {
        if (destroy->reason ==
            static_cast<std::uint8_t>(proto::DestroyReason::KILLED)) {
            device.playExplosion();
        }
    }
}

} // namespace client::flow
