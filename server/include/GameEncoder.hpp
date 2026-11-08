/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameEncoder.hpp
*/

#ifndef GAMEENCODER_HPP_
#define GAMEENCODER_HPP_

#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "Messages.hpp"
#include "Rtype/GameEvents.hpp"

namespace rtype::server {

/**
 * @brief Translates what the simulation produces into protocol packets.
 *
 * The packets have the shape Gateway::queueFromSimulation() expects: a blank
 * header, then records whose rel_id stays at 0 until the reliable channel
 * fills it.
 */
class GameEncoder {
public:
    /**
     * @brief Number of snapshots between two that list the missiles.
     */
    static constexpr std::uint64_t MISSILE_PERIOD = 4;

    [[nodiscard]] static protocol::Message
    toMessage(const game::GameEvent &event);

    /**
     * @brief Turns the events, in order, into packets of reliable records,
     * each within MAX_PACKET_SIZE and MAX_RECORDS.
     */
    [[nodiscard]] static std::vector<std::string>
    encodeEvents(std::span<const game::GameEvent> events);

    /**
     * @brief Number of ticks between two snapshots, at least 1.
     */
    [[nodiscard]] static std::uint64_t
    snapshotInterval(std::uint32_t tickRate,
                     std::uint32_t snapshotRate) noexcept;

    /**
     * @brief Builds the snapshot number index of the game.
     *
     * Priority: ships, then Bydos, in every snapshot, then the missiles,
     * which move in a straight line, every MISSILE_PERIOD snapshots, cut at
     * MAX_SNAPSHOT_ENTITIES.
     */
    [[nodiscard]] static protocol::Snapshot
    makeSnapshot(std::uint64_t tick, std::uint64_t index,
                 std::span<const game::EntityState> states);

    /**
     * @brief Returns a packet holding the single snapshot record.
     */
    [[nodiscard]] static std::string
    encodeSnapshot(const protocol::Snapshot &snapshot);
};

} /* namespace rtype::server */

#endif /* !GAMEENCODER_HPP_ */
