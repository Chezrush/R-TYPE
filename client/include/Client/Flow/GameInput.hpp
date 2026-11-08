/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameInput.hpp
*/

#ifndef GAMEINPUT_HPP_
#define GAMEINPUT_HPP_

#include <cstdint>
#include <string>

#include "Client/Events/ConnectionEvents.hpp"
#include "Client/Flow/IGameDevice.hpp"
#include "Client/Net/ClientSession.hpp"
#include "Client/Settings/ClientSettings.hpp"
#include "Messages.hpp"

namespace client::flow {

/**
 * @brief The protocol input bits for the keys held now.
 *
 * Arrows and WASD move, the bound key shoots (or always, with auto-fire).
 */
[[nodiscard]] std::uint8_t readKeys(const IGameDevice &device,
                                    const settings::ClientSettings &settings);

/** @brief Text shown over the game for a session that is not connected. */
[[nodiscard]] std::string bannerOf(client::net::ClientSession::State state);

/** @brief Why a session that did not reach the game failed. */
[[nodiscard]] events::ConnectionFailure
failureOf(client::net::ClientSession::State state,
          std::uint8_t rejectReason) noexcept;

/** @brief Plays the sound a server message calls for, if any. */
void playSounds(const rtype::protocol::Message &message, IGameDevice &device);

} // namespace client::flow

#endif /* !GAMEINPUT_HPP_ */
