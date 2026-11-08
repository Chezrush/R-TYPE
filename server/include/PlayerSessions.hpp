/*
** EPITECH PROJECT, 2026
** R-TYPE
** File description:
** PlayerSessions
*/

#ifndef PLAYERSESSIONS_HPP_
#define PLAYERSESSIONS_HPP_

#include <cstddef>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <vector>

#include "ServerMessage.hpp"

namespace rtype::server {

/**
 * @brief Connected players and their player slots.
 *
 * Owned by the simulation thread only.
 */
class PlayerSessions {
public:
    /**
     * @brief Gives a free player slot to a new client.
     *
     * @return false if the client already plays or every slot is taken.
     */
    bool join(std::uint32_t clientId);

    /**
     * @brief Same as join(), asking for a given slot.
     *
     * The first free slot is used instead if the one asked for is taken.
     */
    bool joinAt(std::uint32_t clientId, std::uint8_t slot);

    /**
     * @brief Removes the player of the client and frees its slot.
     *
     * Leaving twice is safe: the second call does nothing.
     *
     * @return The player_left messages for the remaining clients, empty if
     * the client was not playing.
     */
    [[nodiscard]] std::vector<OutgoingMessage>
    leave(std::uint32_t clientId, protocol::DisconnectReason reason);

    [[nodiscard]] bool contains(std::uint32_t clientId) const noexcept;

    /**
     * @brief Returns the player slot of a client, if it plays.
     */
    [[nodiscard]] std::optional<std::uint8_t>
    slotOf(std::uint32_t clientId) const noexcept;
    [[nodiscard]] std::size_t size() const noexcept;

private:
    [[nodiscard]] std::optional<std::uint8_t> freeIndex() const noexcept;

    /** @brief Player slot index of each playing client. */
    std::unordered_map<std::uint32_t, std::uint8_t> _players;
};

} /* namespace rtype::server */

#endif /* !PLAYERSESSIONS_HPP_ */
