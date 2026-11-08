/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Connection.cpp
*/

#include "Connection.hpp"

#include <utility>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "MessageCodec.hpp"
#include "PacketBuilder.hpp"
#include "PacketReader.hpp"

namespace rtype::protocol {

Connection::Connection(std::uint32_t session) noexcept
    : _session(session)
{}

std::uint32_t Connection::session() const noexcept
{
    return _session;
}

bool Connection::queueReliable(const Message &message)
{
    if (!isReliable(typeOf(message))) {
        return false;
    }
    return _sender.enqueue(static_cast<std::uint8_t>(typeOf(message)),
                           MessageCodec::encodeBody(message));
}

MessageHeader Connection::nextHeader(std::uint16_t seq) const
{
    const bool acking = _window.hasReceived();
    MessageHeader header;

    header.session = _session;
    header.seq = seq;
    header.flags = acking ? FLAG_ACK_VALID : 0;
    header.ack = acking ? _window.latest() : 0;
    header.ackBits = acking ? _window.seenMask() : 0;
    return header;
}

std::size_t Connection::appendUnreliable(std::string &packet,
                                         std::span<const Message> messages)
{
    std::size_t records = 0;

    for (const Message &message : messages) {
        const bool fits = packet.size() + MessageCodec::recordSize(message) <=
                          MAX_PACKET_SIZE;

        if (records < MAX_RECORDS && fits) {
            MessageCodec::appendMessage(packet, message);
            ++records;
        }
    }
    return records;
}

void Connection::appendDue(std::string &packet, TimePoint now,
                           std::uint16_t seq, std::size_t records)
{
    const auto due =
        _sender.collect(now, _rtt.retransmissionTimeout(), seq,
                        MAX_PACKET_SIZE - packet.size(), MAX_RECORDS - records);

    for (const ReliableRecord &record : due) {
        std::string body;

        ByteWriter(body).writeU16(record.relId).writeBytes(record.payload);
        PacketBuilder::appendRecord(
            packet, static_cast<MessageType>(record.type), body);
    }
}

std::string Connection::buildDatagram(TimePoint now,
                                      std::span<const Message> unreliable)
{
    const std::uint16_t seq = _nextSeq++;
    std::string packet;

    PacketBuilder::writeHeader(packet, nextHeader(seq));
    const std::size_t records = appendUnreliable(packet, unreliable);

    appendDue(packet, now, seq, records);
    _sentTimes[seq % _sentTimes.size()] = SentTime{seq, now, true};
    return packet;
}

bool Connection::reliableInWindow(std::string_view records) const
{
    while (const auto record = PacketReader::readRecord(records)) {
        if (!isReliable(record->type)) {
            continue;
        }
        ByteReader reader(record->body);

        if (!_receiver.inWindow(reader.readU16())) {
            return false;
        }
    }
    return true;
}

void Connection::applyAck(const MessageHeader &header, TimePoint now)
{
    if ((header.flags & FLAG_ACK_VALID) == 0) {
        return;
    }
    _sender.onAck(header.ack, header.ackBits);
    SentTime &sent = _sentTimes[header.ack % _sentTimes.size()];

    if (sent.pending && sent.seq == header.ack) {
        sent.pending = false;
        _rtt.addSample(std::chrono::duration_cast<Duration>(now - sent.at));
    }
}

void Connection::deliverReliable(std::vector<ReliableRecord> &released,
                                 ReceivedDatagram &result)
{
    for (const ReliableRecord &record : released) {
        auto message = MessageCodec::decodeBody(record.type, record.payload);

        if (message) {
            result.reliable.push_back(std::move(*message));
        }
    }
    released.clear();
}

void Connection::deliverRecord(const Record &record, ReceivedDatagram &result)
{
    result.hadRecords = true;
    if (!isReliable(record.type)) {
        auto message = MessageCodec::decodeBody(record.type, record.body);

        if (message) {
            result.unreliable.push_back(std::move(*message));
        }
        return;
    }
    std::vector<ReliableRecord> released;
    ByteReader reader(record.body);
    const std::uint16_t relId = reader.readU16();

    _receiver.accept(
        ReliableRecord{relId, record.type,
                       std::string(reader.readBytes(reader.remaining()))},
        released);
    deliverReliable(released, result);
}

void Connection::deliver(std::string_view records, ReceivedDatagram &result)
{
    while (const auto record = PacketReader::readRecord(records)) {
        deliverRecord(*record, result);
    }
}

DatagramStatus Connection::screen(const MessageHeader &header,
                                  std::string_view records) const
{
    if (header.session != _session) {
        return DatagramStatus::WRONG_SESSION;
    }
    const SeqVerdict verdict = _window.check(header.seq);

    if (verdict == SeqVerdict::DUPLICATE) {
        return DatagramStatus::DUPLICATE;
    }
    if (verdict == SeqVerdict::TOO_OLD) {
        return DatagramStatus::TOO_OLD;
    }
    return reliableInWindow(records) ? DatagramStatus::ACCEPTED
                                     : DatagramStatus::RELIABLE_OUT_OF_WINDOW;
}

ReceivedDatagram Connection::receive(std::string_view datagram, TimePoint now)
{
    ReceivedDatagram result;
    const auto header = PacketReader::parseHeader(datagram);

    if (!header || !PacketReader::isValid(datagram)) {
        return result;
    }
    const std::string_view records = PacketReader::recordsOf(datagram);

    result.status = screen(*header, records);
    if (result.status != DatagramStatus::ACCEPTED) {
        return result;
    }
    _window.commit(header->seq);
    _lastReceived = now;
    applyAck(*header, now);
    deliver(records, result);
    return result;
}

bool Connection::hasDueReliable(TimePoint now) const noexcept
{
    return _sender.hasDue(now, _rtt.retransmissionTimeout());
}

const RttEstimator &Connection::rtt() const noexcept
{
    return _rtt;
}

TimePoint Connection::lastReceived() const noexcept
{
    return _lastReceived;
}

std::size_t Connection::pendingReliable() const noexcept
{
    return _sender.pending();
}

} /* namespace rtype::protocol */
