/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ReliableReceiver.hpp
*/

#ifndef RELIABLERECEIVER_HPP_
#define RELIABLERECEIVER_HPP_

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "ReliableRecord.hpp"

namespace rtype::protocol {

enum class ReliableVerdict {
    DELIVERED,
    BUFFERED,
    DUPLICATE,
    OUT_OF_WINDOW
};

/**
 * @brief Delivers the reliable records exactly once and in rel_id order.
 */
class ReliableReceiver {
public:
    /**
     * @brief Returns false if rel_id is beyond the receive window, which
     * means the peer is broken and the whole datagram must be dropped.
     */
    [[nodiscard]] bool inWindow(std::uint16_t relId) const noexcept;

    /**
     * @brief Accepts a record, appends to delivered every record that can
     * now be handed to the game, in order.
     */
    ReliableVerdict accept(ReliableRecord record,
                           std::vector<ReliableRecord> &delivered);

    [[nodiscard]] std::uint16_t nextExpected() const noexcept;
    [[nodiscard]] std::size_t buffered() const noexcept;

private:
    std::uint16_t _next = 0;
    std::unordered_map<std::uint16_t, ReliableRecord> _buffer;
};

} /* namespace rtype::protocol */

#endif /* !RELIABLERECEIVER_HPP_ */
