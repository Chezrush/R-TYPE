/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameBridgeTests.cpp
*/

#include <string>
#include <vector>

#include "FixedPoint.hpp"
#include "GameBridge.hpp"
#include "InputQueue.hpp"
#include "MessageCodec.hpp"
#include "PacketReader.hpp"
#include "TestSupport.hpp"

using namespace rtype::server;
using namespace rtype::protocol;

namespace {

std::vector<Message> decode(const std::string &payload)
{
    std::vector<Message> messages;
    std::string_view records = PacketReader::recordsOf(payload);

    while (const auto record = PacketReader::readRecord(records)) {
        const auto body = isReliable(record->type)
                              ? record->body.substr(REL_ID_SIZE)
                              : record->body;

        if (auto message = MessageCodec::decodeBody(record->type, body)) {
            messages.push_back(std::move(*message));
        }
    }
    return messages;
}

std::vector<Message> messagesFor(const std::vector<OutgoingMessage> &out,
                                 std::uint32_t clientId)
{
    std::vector<Message> messages;

    for (const OutgoingMessage &message : out) {
        if (message.clientId == clientId) {
            for (Message &m : decode(message.payload)) {
                messages.push_back(std::move(m));
            }
        }
    }
    return messages;
}

template <typename T>
std::vector<T> only(const std::vector<Message> &messages)
{
    std::vector<T> found;

    for (const Message &message : messages) {
        if (const auto *item = std::get_if<T>(&message)) {
            found.push_back(*item);
        }
    }
    return found;
}

rtype::game::GameRules rules()
{
    rtype::game::GameRules value;

    value.tickRate = 60;
    return value;
}

void testInputQueueAppliesOneSamplePerTick()
{
    InputQueue queue;

    queue.push(Input{1, {0x04, 0x08, 0x00}});
    CHECK(queue.next() == 0x04);
    CHECK(queue.applied() == 1);
    CHECK(queue.next() == 0x08);
    CHECK(queue.next() == 0x00);
    CHECK(queue.applied() == 3);
}

void testInputQueueHoldsTheLastKeys()
{
    InputQueue queue;

    queue.push(Input{1, {0x10}});
    CHECK(queue.next() == 0x10);
    CHECK(queue.next() == 0x10);
    CHECK(queue.applied() == 1);
}

void testInputQueueIgnoresSamplesAlreadySeen()
{
    InputQueue queue;

    queue.push(Input{1, {0x01, 0x02}});
    queue.push(Input{1, {0x01, 0x02, 0x04}});
    CHECK(queue.pending() == 3);
    queue.push(Input{2, {0x02}});
    CHECK(queue.pending() == 3);
}

void testInputQueueIsBounded()
{
    InputQueue queue;

    queue.push(Input{1, std::vector<std::uint8_t>(8, 1)});
    queue.push(Input{9, std::vector<std::uint8_t>(8, 1)});
    CHECK(queue.pending() == MAX_INPUTS);
}

void testInputQueueIdsWrapAround()
{
    InputQueue queue;

    queue.push(Input{65534, {1, 2, 4, 8}});
    for (int i = 0; i < 4; ++i) {
        static_cast<void>(queue.next());
    }
    CHECK(queue.applied() == 1);
    queue.push(Input{2, {16}});
    CHECK(queue.next() == 16);
    CHECK(queue.applied() == 2);
}

void testNewClientIsSpawnedForEveryone()
{
    GameBridge bridge(rules(), 1, 20);

    CHECK(bridge.addClient(1, 0).empty());
    CHECK(bridge.addClient(2, 1).size() == 1);
    const auto out = bridge.step();
    const auto spawns = only<Spawn>(messagesFor(out, 1));

    CHECK(spawns.size() == 2);
    CHECK(only<Spawn>(messagesFor(out, 2)).size() == 2);
    CHECK(spawns[0].kind == static_cast<std::uint8_t>(EntityKind::PLAYER_SHIP));
    CHECK(spawns[0].owner == 0 || spawns[0].owner == 1);
}

void testLateJoinerReceivesTheWorld()
{
    GameBridge bridge(rules(), 1, 20);

    static_cast<void>(bridge.addClient(1, 0));
    static_cast<void>(bridge.step());
    const auto world = bridge.addClient(2, 1);

    CHECK(world.size() == 1);
    CHECK(world[0].clientId == 2);
    CHECK(only<Spawn>(decode(world[0].payload)).size() == 1);
}

void testSnapshotsFollowTheRateAndCarryTheInputAck()
{
    GameBridge bridge(rules(), 1, 20);
    int snapshots = 0;
    std::uint16_t ack = 0;

    static_cast<void>(bridge.addClient(1, 0));
    bridge.onInput(1, Input{1, {KEY_RIGHT}});
    for (int tick = 0; tick < 60; ++tick) {
        for (const Snapshot &s :
             only<Snapshot>(messagesFor(bridge.step(), 1))) {
            ++snapshots;
            ack = s.inputAck;
        }
    }
    CHECK(snapshots == 20);
    CHECK(ack == 1);
}

void testHeldKeysMoveTheShip()
{
    GameBridge bridge(rules(), 1, 20);
    std::int16_t firstX = 0;
    std::int16_t lastX = 0;

    static_cast<void>(bridge.addClient(1, 0));
    bridge.onInput(1, Input{1, {KEY_RIGHT}});
    for (int tick = 0; tick < 30; ++tick) {
        for (const Snapshot &s :
             only<Snapshot>(messagesFor(bridge.step(), 1))) {
            firstX = firstX == 0 ? s.entities[0].x : firstX;
            lastX = s.entities[0].x;
        }
    }
    CHECK(lastX > firstX);
}

void testRemovedClientIsDestroyedForTheOthers()
{
    GameBridge bridge(rules(), 1, 20);

    static_cast<void>(bridge.addClient(1, 0));
    static_cast<void>(bridge.addClient(2, 1));
    static_cast<void>(bridge.step());
    bridge.removeClient(1);
    const auto out = bridge.step();

    CHECK(bridge.clientCount() == 1);
    CHECK(only<Destroy>(messagesFor(out, 2)).size() == 1);
    CHECK(messagesFor(out, 1).empty());
}

void testInputOfAnUnknownClientIsIgnored()
{
    GameBridge bridge(rules(), 1, 20);

    bridge.onInput(42, Input{1, {1}});
    CHECK(bridge.clientCount() == 0);
}

} // namespace

int main()
{
    testInputQueueAppliesOneSamplePerTick();
    testInputQueueHoldsTheLastKeys();
    testInputQueueIgnoresSamplesAlreadySeen();
    testInputQueueIsBounded();
    testInputQueueIdsWrapAround();
    testNewClientIsSpawnedForEveryone();
    testLateJoinerReceivesTheWorld();
    testSnapshotsFollowTheRateAndCarryTheInputAck();
    testHeldKeysMoveTheShip();
    testRemovedClientIsDestroyedForTheOthers();
    testInputOfAnUnknownClientIsIgnored();
    return TEST_RESULT();
}
