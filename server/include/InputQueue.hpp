/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** InputQueue.hpp
*/

#ifndef INPUTQUEUE_HPP_
#define INPUTQUEUE_HPP_

#include <cstddef>
#include <cstdint>
#include <deque>
#include <utility>

#include "Messages.hpp"

namespace rtype::server {

/**
 * @brief Input samples of one client, consumed at the pace of the server.
 *
 * Samples are applied in input_id order, at most one per tick. A sample
 * already queued is ignored, and so is anything beyond MAX_INPUTS pending
 * samples: a client cannot run faster than the server clock. The keys of
 * the previous sample are held when nothing is pending.
 */
class InputQueue {
public:
    /**
     * @brief Queues the samples of an input message that were not seen yet.
     */
    void push(const protocol::Input &input);

    /**
     * @brief Takes the next sample and returns the keys to hold this tick.
     */
    std::uint8_t next();

    /**
     * @brief Returns the id of the last sample applied, 0 before any.
     */
    [[nodiscard]] std::uint16_t applied() const noexcept;

    [[nodiscard]] std::size_t pending() const noexcept;

private:
    std::deque<std::pair<std::uint16_t, std::uint8_t>> _pending;
    std::uint16_t _lastQueued = 0;
    std::uint16_t _applied = 0;
    std::uint8_t _held = 0;
};

} /* namespace rtype::server */

#endif /* !INPUTQUEUE_HPP_ */
