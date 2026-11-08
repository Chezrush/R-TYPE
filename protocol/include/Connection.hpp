/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Connection.hpp
*/

#ifndef CONNECTION_HPP_
#define CONNECTION_HPP_

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "Messages.hpp"
#include "ReceiveWindow.hpp"
#include "ReliableReceiver.hpp"
#include "ReliableSender.hpp"
#include "RttEstimator.hpp"
#include "Timing.hpp"

namespace rtype::protocol {

enum class DatagramStatus {
    ACCEPTED,
    INVALID,
    WRONG_SESSION,
    DUPLICATE,
    TOO_OLD,
    RELIABLE_OUT_OF_WINDOW
};

/**
 * @brief What a datagram delivered to the game.
 *
 * reliable holds the messages released by the reliable channel, in order
 * and exactly once. Records of an unknown type are skipped.
 */
struct ReceivedDatagram {
    DatagramStatus status = DatagramStatus::INVALID;
    bool hadRecords = false;
    std::vector<Message> unreliable;
    std::vector<Message> reliable;
};

/**
 * @brief One side of an established connection: datagram numbering,
 * acknowledgements, duplicate detection and the reliable channel.
 *
 * It does no I/O and reads no clock: the datagrams and the time are passed
 * in, so the whole behavior can be tested without a network.
 */
class Connection {
public:
    explicit Connection(std::uint32_t session = 0) noexcept;

    [[nodiscard]] std::uint32_t session() const noexcept;

    /**
     * @brief Queues a message for the reliable channel.
     *
     * @return false if the message is not a reliable type or if the queue is
     * full. A full queue means the peer cannot keep up: close the
     * connection with DisconnectReason::QUEUE_OVERFLOW.
     */
    bool queueReliable(const Message &message);

    /**
     * @brief Builds the next datagram.
     *
     * It carries the unreliable messages that fit, then the reliable
     * records that are due, and always the acknowledgements.
     */
    [[nodiscard]] std::string
    buildDatagram(TimePoint now, std::span<const Message> unreliable = {});

    /**
     * @brief Validates a datagram and commits its effects (window,
     * acknowledgements, reliable channel) only if it is fully valid.
     */
    [[nodiscard]] ReceivedDatagram receive(std::string_view datagram,
                                           TimePoint now);

    /**
     * @brief Returns true if a reliable record must be sent now, for the
     * first time or again.
     */
    [[nodiscard]] bool hasDueReliable(TimePoint now) const noexcept;

    [[nodiscard]] const RttEstimator &rtt() const noexcept;
    [[nodiscard]] TimePoint lastReceived() const noexcept;
    [[nodiscard]] std::size_t pendingReliable() const noexcept;

private:
    struct SentTime {
        std::uint16_t seq = 0;
        TimePoint at;
        bool pending = false;
    };

    [[nodiscard]] bool reliableInWindow(std::string_view records) const;
    void applyAck(const MessageHeader &header, TimePoint now);
    void deliver(std::string_view records, ReceivedDatagram &result);
    void deliverReliable(std::vector<ReliableRecord> &released,
                         ReceivedDatagram &result) const;

    std::uint32_t _session;
    std::uint16_t _nextSeq = 0;
    ReceiveWindow _window;
    ReliableSender _sender;
    ReliableReceiver _receiver;
    RttEstimator _rtt;
    TimePoint _lastReceived{};
    std::array<SentTime, 64> _sentTimes{};
};

} /* namespace rtype::protocol */

#endif /* !CONNECTION_HPP_ */
