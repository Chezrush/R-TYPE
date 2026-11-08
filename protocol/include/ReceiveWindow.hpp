/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ReceiveWindow.hpp
*/

#ifndef RECEIVEWINDOW_HPP_
#define RECEIVEWINDOW_HPP_

#include <cstdint>

namespace rtype::protocol {

enum class SeqVerdict : std::uint8_t {
    ACCEPT,
    DUPLICATE,
    TOO_OLD
};

/**
 * @brief Sliding window over the seq of the datagrams received from a peer.
 *
 * check() does not change anything: a datagram is only committed once it
 * has been fully validated. latest() and seenMask() are the ack and
 * ack_bits to put in the header of the datagrams sent back.
 */
class ReceiveWindow {
public:
    [[nodiscard]] SeqVerdict check(std::uint16_t seq) const noexcept;
    void commit(std::uint16_t seq) noexcept;

    [[nodiscard]] bool hasReceived() const noexcept;
    [[nodiscard]] std::uint16_t latest() const noexcept;
    [[nodiscard]] std::uint32_t seenMask() const noexcept;

private:
    void markSeen(std::uint16_t age) noexcept;
    void advance(std::uint16_t seq) noexcept;

    bool _started = false;
    std::uint16_t _latest = 0;
    std::uint32_t _seen = 0;
};

} /* namespace rtype::protocol */

#endif /* !RECEIVEWINDOW_HPP_ */
