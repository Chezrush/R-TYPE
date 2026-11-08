/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ReliableSender.cpp
*/

#include "ReliableSender.hpp"

#include <algorithm>
#include <utility>

#include "Protocol.hpp"
#include "SerialNumber.hpp"

namespace rtype::protocol {

namespace {

constexpr std::size_t MAX_LOGGED_DATAGRAMS = 64;
constexpr unsigned FAST_RETRANSMIT_ACKS = 3;

} // namespace

bool ReliableSender::enqueue(std::uint8_t type, std::string payload)
{
    if (!hasRoomToQueue()) {
        return false;
    }
    Entry entry;

    entry.record = ReliableRecord{_nextRelId++, type, std::move(payload)};
    _entries.push_back(std::move(entry));
    return true;
}

bool ReliableSender::inSendWindow(const Entry &entry) const noexcept
{
    const std::uint16_t offset =
        distance(entry.record.relId, _entries.front().record.relId);

    return offset < MAX_OUTSTANDING_RELIABLE;
}

bool ReliableSender::isDue(const Entry &entry, TimePoint now,
                           Duration timeout) noexcept
{
    if (!entry.sent) {
        return true;
    }
    return now - entry.lastSent >= timeout ||
           entry.newerAcks >= FAST_RETRANSMIT_ACKS;
}

std::vector<ReliableRecord>
ReliableSender::gather(TimePoint now, Duration timeout, std::size_t maxBytes,
                       std::size_t maxRecords, SentDatagram &datagram)
{
    std::vector<ReliableRecord> taken;

    for (Entry &entry : _entries) {
        const std::size_t size =
            RECORD_PREFIX_SIZE + REL_ID_SIZE + entry.record.payload.size();
        if (!inSendWindow(entry) || taken.size() >= maxRecords ||
            size > maxBytes) {
            break;
        }
        if (isDue(entry, now, timeout)) {
            entry = Entry{entry.record, true, now, datagram.seq, 0};
            taken.push_back(entry.record);
            datagram.relIds.push_back(entry.record.relId);
            maxBytes -= size;
        }
    }
    return taken;
}

std::vector<ReliableRecord>
ReliableSender::collect(TimePoint now, Duration timeout, std::uint16_t seq,
                        std::size_t maxBytes, std::size_t maxRecords)
{
    SentDatagram datagram{seq, {}};
    auto taken = gather(now, timeout, maxBytes, maxRecords, datagram);

    _log.push_back(std::move(datagram));
    if (_log.size() > MAX_LOGGED_DATAGRAMS) {
        _log.pop_front();
    }
    return taken;
}

void ReliableSender::remove(std::uint16_t relId)
{
    std::erase_if(_entries, [relId](const Entry &entry) {
        return entry.record.relId == relId;
    });
}

void ReliableSender::acknowledge(std::uint16_t seq)
{
    const auto logged = std::find_if(
        _log.begin(), _log.end(),
        [seq](const SentDatagram &datagram) { return datagram.seq == seq; });

    if (logged == _log.end()) {
        return;
    }
    for (const std::uint16_t relId : logged->relIds) {
        remove(relId);
    }
    _log.erase(logged);
    for (Entry &entry : _entries) {
        if (entry.sent && isNewer(seq, entry.lastSeq)) {
            ++entry.newerAcks;
        }
    }
}

void ReliableSender::onAck(std::uint16_t ack, std::uint32_t ackBits)
{
    acknowledge(ack);
    for (std::uint16_t bit = 0; bit < 32; ++bit) {
        if ((ackBits >> bit) & 1U) {
            acknowledge(static_cast<std::uint16_t>(ack - 1U - bit));
        }
    }
}

bool ReliableSender::hasDue(TimePoint now, Duration timeout) const noexcept
{
    return std::any_of(_entries.begin(), _entries.end(),
                       [this, now, timeout](const Entry &entry) {
                           return inSendWindow(entry) &&
                                  isDue(entry, now, timeout);
                       });
}

std::size_t ReliableSender::pending() const noexcept
{
    return _entries.size();
}

std::size_t ReliableSender::outstanding() const noexcept
{
    return static_cast<std::size_t>(
        std::count_if(_entries.begin(), _entries.end(),
                      [](const Entry &entry) { return entry.sent; }));
}

bool ReliableSender::hasRoomToQueue() const noexcept
{
    return _entries.size() < MAX_RELIABLE_QUEUE;
}

} /* namespace rtype::protocol */
