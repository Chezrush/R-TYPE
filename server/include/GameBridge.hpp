/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameBridge.hpp
*/

#ifndef GAMEBRIDGE_HPP_
#define GAMEBRIDGE_HPP_

#include <cstdint>
#include <map>
#include <vector>

#include "InputQueue.hpp"
#include "Messages.hpp"
#include "Rtype/GameRules.hpp"
#include "Rtype/Simulation.hpp"
#include "ServerMessage.hpp"

namespace rtype::server {

/**
 * @brief Connects the game simulation to the protocol messages.
 *
 * It owns the Simulation, feeds it the input of every client, and turns its
 * events and states into the reliable events and the snapshots the clients
 * receive. It does no I/O and is owned by the simulation thread.
 */
class GameBridge {
public:
    GameBridge(const game::GameRules &rules, std::uint32_t seed,
               std::uint8_t snapshotRate);

    /**
     * @brief Adds the ship of a client.
     *
     * @return The spawn of every entity already alive, for the newcomer.
     */
    [[nodiscard]] std::vector<OutgoingMessage> addClient(std::uint32_t clientId,
                                                         std::uint8_t slot);

    void removeClient(std::uint32_t clientId);

    void onInput(std::uint32_t clientId, const protocol::Input &input);

    /**
     * @brief Advances the game by one tick.
     *
     * @return The events for every client, and a snapshot for each of them
     * every 1 / snapshotRate second.
     */
    [[nodiscard]] std::vector<OutgoingMessage> step();

    [[nodiscard]] std::size_t clientCount() const noexcept;

private:
    struct Client {
        std::uint8_t slot = 0;
        InputQueue inputs;
    };

    [[nodiscard]] std::vector<OutgoingMessage>
    broadcast(const std::vector<protocol::Message> &events) const;
    [[nodiscard]] std::vector<OutgoingMessage> snapshots() const;

    game::Simulation _simulation;
    std::uint32_t _snapshotEvery;
    std::map<std::uint32_t, Client> _clients;
};

} /* namespace rtype::server */

#endif /* !GAMEBRIDGE_HPP_ */
