/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameEncoderTests.cpp
*/

#include <algorithm>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "GameEncoder.hpp"
#include "MessageCodec.hpp"
#include "PacketReader.hpp"
#include "TestSupport.hpp"

using namespace rtype;
using namespace rtype::server;
using namespace rtype::protocol;

namespace {

/**
 * @brief Decodes every record of a packet, the rel_id of reliable ones
 * removed. A record that does not decode gives std::nullopt.
 */
std::vector<std::optional<Message>> decodePacket(std::string_view packet)
{
    std::string_view records = PacketReader::recordsOf(packet);
    std::vector<std::optional<Message>> messages;

    while (const auto record = PacketReader::readRecord(records)) {
        const auto body = isReliable(record->type)
                              ? record->body.substr(REL_ID_SIZE)
                              : record->body;
        messages.push_back(MessageCodec::decodeBody(record->type, body));
    }
    return messages;
}

game::EntityState entity(std::uint16_t netId, game::EntityKind kind)
{
    return {netId, kind, {1.5F, 2.0F}};
}

std::vector<game::GameEvent> sampleEvents()
{
    return {game::SpawnEvent{7,
                             game::EntityKind::PLAYER_MISSILE,
                             2,
                             1,
                             {10.5F, 20.25F},
                             {-300.0F, 0.0F}},
            game::DestroyEvent{7, game::DestroyReason::KILLED, 1},
            game::ScoreEvent{1, 4200},
            game::GameStateEvent{game::GamePhase::PLAYING, 3}};
}

void testEventsRoundTrip()
{
    const auto packets = GameEncoder::encodeEvents(sampleEvents());
    const std::vector<std::optional<Message>> expected = {
        Spawn{7, 3, 2, 1, 168, 324, -4800, 0}, Destroy{7, 1, 1}, Score{1, 4200},
        GameState{1, 3}};

    CHECK(packets.size() == 1);
    for (const std::string &packet : packets) {
        CHECK(PacketReader::isValid(packet));
        CHECK(decodePacket(packet) == expected);
    }
}

void testEveryEventIsReliable()
{
    for (const game::GameEvent &event : sampleEvents()) {
        CHECK(isReliable(typeOf(GameEncoder::toMessage(event))));
    }
}

void testManyEventsAreSplitIntoValidPackets()
{
    const std::vector<game::GameEvent> events(500, sampleEvents().front());
    const auto packets = GameEncoder::encodeEvents(events);
    std::size_t total = 0;

    CHECK(packets.size() > 1);
    for (const std::string &packet : packets) {
        CHECK(PacketReader::isValid(packet));
        total += decodePacket(packet).size();
    }
    CHECK(total == events.size());
}

void testNoEventGivesNoPacket()
{
    CHECK(GameEncoder::encodeEvents({}).empty());
}

void testSnapshotInterval()
{
    CHECK(GameEncoder::snapshotInterval(60, 20) == 3);
    CHECK(GameEncoder::snapshotInterval(20, 20) == 1);
    CHECK(GameEncoder::snapshotInterval(10, 20) == 1);
    CHECK(GameEncoder::snapshotInterval(60, 0) == 60);
}

void testSnapshotRoundTrips()
{
    const std::vector<game::EntityState> states = {
        entity(4, game::EntityKind::BYDOS),
        entity(2, game::EntityKind::PLAYER_SHIP)};
    const auto snapshot = GameEncoder::makeSnapshot(90, 30, states);
    const std::string packet = GameEncoder::encodeSnapshot(snapshot);
    const std::optional<Message> expected =
        Snapshot{90, 0, {{2, 24, 32}, {4, 24, 32}}};

    CHECK(PacketReader::isValid(packet));
    CHECK(decodePacket(packet) == std::vector{expected});
}

void testMissilesOnlyEveryFourthSnapshot()
{
    const std::vector<game::EntityState> states = {
        entity(1, game::EntityKind::PLAYER_MISSILE),
        entity(2, game::EntityKind::BYDOS_MISSILE),
        entity(3, game::EntityKind::BYDOS)};

    CHECK(GameEncoder::makeSnapshot(0, 4, states).entities.size() == 3);
    CHECK(GameEncoder::makeSnapshot(0, 5, states).entities.size() == 1);
}

void testSnapshotKeepsTheShipsWhenFull()
{
    std::vector<game::EntityState> states(
        200, entity(9, game::EntityKind::BYDOS_MISSILE));

    states.push_back(entity(1, game::EntityKind::PLAYER_SHIP));
    const auto snapshot = GameEncoder::makeSnapshot(0, 0, states);
    const std::string packet = GameEncoder::encodeSnapshot(snapshot);

    CHECK(snapshot.entities.size() == MAX_SNAPSHOT_ENTITIES);
    CHECK(std::ranges::count(snapshot.entities, 1, &EntityState::netId) == 1);
    CHECK(PacketReader::isValid(packet));
}

void testEmptyWorldGivesValidSnapshot()
{
    const auto snapshot = GameEncoder::makeSnapshot(3, 1, {});

    CHECK(snapshot.entities.empty());
    CHECK(PacketReader::isValid(GameEncoder::encodeSnapshot(snapshot)));
}

} // namespace

int main()
{
    testEventsRoundTrip();
    testEveryEventIsReliable();
    testManyEventsAreSplitIntoValidPackets();
    testNoEventGivesNoPacket();
    testSnapshotInterval();
    testSnapshotRoundTrips();
    testMissilesOnlyEveryFourthSnapshot();
    testSnapshotKeepsTheShipsWhenFull();
    testEmptyWorldGivesValidSnapshot();
    return TEST_RESULT();
}
