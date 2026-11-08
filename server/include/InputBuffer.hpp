/*
** EPITECH PROJECT, 2026
** R-TYPE
** File description:
** InputBuffer
*/

#ifndef INPUTBUFFER_HPP_
#define INPUTBUFFER_HPP_

#include <array>
#include <cstdint>
#include <optional>

#include "Messages.hpp"

namespace rtype::server {

/**
 * @brief Input samples of one player waiting to be applied, by input_id.
 *
 * At most MAX_INPUTS samples newer than the last applied one are kept, the
 * others are dropped: a client cannot run ahead of the server clock.
 */
class InputBuffer {
public:
    /**
     * @brief Keeps the samples of the input not applied yet.
     *
     * The first input received sets where the numbering starts.
     */
    void push(const protocol::Input &input);

    /**
     * @brief Takes the oldest pending sample, skipping the missing ids.
     *
     * @return Its keys, or nothing if no sample is pending.
     */
    [[nodiscard]] std::optional<std::uint8_t> pop();

    /**
     * @brief Returns the input_id of the last sample taken by pop(), the
     * input_ack of the snapshots.
     */
    [[nodiscard]] std::uint16_t lastApplied() const noexcept;

private:
    void store(std::uint16_t id, std::uint8_t keys);

    /** @brief Sample of id _lastApplied + 1 + index. */
    std::array<std::optional<std::uint8_t>, protocol::MAX_INPUTS> _pending{};
    std::uint16_t _lastApplied = 0;
    bool _started = false;
};

} /* namespace rtype::server */

#endif /* !INPUTBUFFER_HPP_ */
