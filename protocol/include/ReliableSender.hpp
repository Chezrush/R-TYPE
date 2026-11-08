/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ReliableSender.hpp
*/

#ifndef RELIABLESENDER_HPP_
#define RELIABLESENDER_HPP_

#include <cstddef>
#include <cstdint>
#include <deque>
#include <string>
#include <vector>

#include "ReliableRecord.hpp"
#include "Timing.hpp"

namespace rtype::protocol {

/**
 * @brief Keeps every reliable record until a datagram that carried it is
 * acknowledged, and sends it again when that datagram looks lost.
 *
 * A datagram is lost when it is not acknowledged after the retransmission
 * timeout, or when three newer datagrams were acknowledged first. A
 * retransmitted record keeps its rel_id.
 */
class ReliableSender {
public:
    /**
     * @brief Queues a record and gives it the next rel_id.
     *
     * @return false, and refuses the record, when the queue is full: the
     * peer cannot keep up and the connection must be closed.
     */
    bool enqueue(std::uint8_t type, std::string payload);

    /**
     * @brief Takes the records to put in the datagram numbered seq.
     *
     * @param maxBytes Room left in the datagram for whole records, prefix
     * and rel_id included.
     * @param maxRecords Number of records the datagram can still hold.
     */
    [[nodiscard]] std::vector<ReliableRecord>
    collect(TimePoint now, Duration timeout, std::uint16_t seq,
            std::size_t maxBytes, std::size_t maxRecords);

    /**
     * @brief Applies the ack and ack_bits of a received datagram.
     */
    void onAck(std::uint16_t ack, std::uint32_t ackBits);

    /**
     * @brief Returns true if collect() would take at least one record now.
     */
    [[nodiscard]] bool hasDue(TimePoint now, Duration timeout) const noexcept;

    [[nodiscard]] std::size_t pending() const noexcept;
    [[nodiscard]] std::size_t outstanding() const noexcept;
    [[nodiscard]] bool hasRoomToQueue() const noexcept;

private:
    struct Entry {
        ReliableRecord record;
        bool sent = false;
        TimePoint lastSent;
        std::uint16_t lastSeq = 0;
        unsigned newerAcks = 0;
    };

    struct SentDatagram {
        std::uint16_t seq = 0;
        std::vector<std::uint16_t> relIds;
    };

    [[nodiscard]] bool isDue(const Entry &entry, TimePoint now,
                             Duration timeout) const noexcept;
    [[nodiscard]] bool inSendWindow(const Entry &entry) const noexcept;
    void acknowledge(std::uint16_t seq);
    void remove(std::uint16_t relId);

    std::deque<Entry> _entries;
    std::deque<SentDatagram> _log;
    std::uint16_t _nextRelId = 0;
};

} /* namespace rtype::protocol */

#endif /* !RELIABLESENDER_HPP_ */
