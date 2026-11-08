/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Gateway.hpp
*/

#ifndef GATEWAY_HPP_
#define GATEWAY_HPP_

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "Connection.hpp"
#include "NetStats.hpp"
#include "RateLimiter.hpp"
#include "ServerHandshake.hpp"
#include "ServerMessage.hpp"

namespace rtype::server {

using Clock = protocol::Clock;
using TimePoint = protocol::TimePoint;

/**
 * @brief Parameters sent to the clients in welcome.
 */
struct GatewayConfig {
    std::uint8_t maxPlayers = 4;
    std::uint8_t tickRate = 60;
    std::uint8_t snapshotRate = 20;
    std::uint16_t worldWidth = 1280;
    std::uint16_t worldHeight = 720;
    std::uint16_t shipSpeed = 300;
    protocol::Duration keepalive = std::chrono::seconds(1);
};

/**
 * @brief Traffic and latency of one client.
 */
struct ClientStats {
    std::uint32_t clientId = 0;
    NetCounters traffic;
    std::chrono::milliseconds rtt{0};
};

/**
 * @brief Copy of the network statistics at one instant.
 */
struct GatewayStats {
    NetCounters totals;
    std::vector<ClientStats> clients;

    [[nodiscard]] std::chrono::milliseconds averageRtt() const;
};

/**
 * @brief The protocol seen from the server network thread.
 *
 * It runs the handshake of the addresses with no connection, owns the
 * Connection of every client, and turns what the simulation produces into
 * datagrams. It never touches a socket: datagrams come in through
 * onDatagram() and leave through the Send callback, so it can be tested
 * without a network. All methods are thread-safe.
 *
 * A client is identified on the wire by its address, written "ip|port".
 */
class Gateway {
public:
    using Send = std::function<void(const std::string &, const std::string &)>;
    using Deliver = std::function<bool(IncomingMessage)>;
    using Drop = std::function<void(std::uint32_t, protocol::DisconnectReason)>;

    Gateway(GatewayConfig config, const protocol::SipKey &secret, Send send,
            Deliver deliver, Drop drop);

    /**
     * @brief Handles one datagram received from address.
     *
     * @param nowSeconds Wall time in seconds, used to age the cookies.
     */
    void onDatagram(const std::string &address, std::string_view datagram,
                    TimePoint now, std::uint64_t nowSeconds);

    /**
     * @brief Queues what the simulation sends to a client.
     *
     * The payload holds records: reliable ones travel by the reliable
     * channel of the client, the others in its next datagram.
     */
    void queueFromSimulation(const OutgoingMessage &message);

    /**
     * @brief Sends the pending messages, the retransmissions, the
     * acknowledgements and the keepalives that are due.
     */
    void flush(TimePoint now);

    /**
     * @brief Forgets a client that left, without telling it.
     */
    void remove(std::uint32_t clientId);

    /**
     * @brief Tells every client the server stops, then forgets them.
     */
    void closeAll(protocol::DisconnectReason reason, TimePoint now);

    [[nodiscard]] std::size_t clientCount() const;

    /**
     * @brief Returns a consistent copy of the counters.
     *
     * Totals keep growing after a client leaves, per-client ones do not.
     */
    [[nodiscard]] GatewayStats stats() const;

private:
    struct Remote {
        std::uint32_t clientId = 0;
        std::uint8_t slot = 0;
        std::uint64_t cookie = 0;
        std::string address;
        std::string name;
        protocol::Connection connection;
        TimePoint lastSent;
        TimePoint invalidSince;
        unsigned invalidCount = 0;
        bool ackDue = false;
        NetCounters traffic;
        std::vector<protocol::Message> unreliable;
    };

    void onStranger(const std::string &address, std::string_view datagram,
                    TimePoint now, std::uint64_t nowSeconds);
    void onClient(Remote &remote, std::string_view datagram, TimePoint now,
                  std::uint64_t nowSeconds);
    void onHandshakeRetry(Remote &remote, std::string_view datagram,
                          TimePoint now, std::uint64_t nowSeconds);
    [[nodiscard]] bool queueRecord(Remote &remote,
                                   const protocol::Record &record);
    void onResult(Remote &remote, const protocol::ReceivedDatagram &result,
                  std::string_view datagram, TimePoint now);
    void accept(const std::string &address, const protocol::Hello &hello,
                TimePoint now);
    void sendWelcome(Remote &remote, TimePoint now);
    void introduce(Remote &newcomer);
    void countInvalid(Remote &remote, TimePoint now);
    void close(Remote &remote, protocol::DisconnectReason reason, TimePoint now,
               bool notifySimulation);
    void transmit(const std::string &address, const std::string &datagram,
                  Remote *remote);
    void sendNow(Remote &remote, TimePoint now);
    [[nodiscard]] static std::string
    goodbyeDatagram(Remote &remote, protocol::DisconnectReason reason,
                    TimePoint now);
    [[nodiscard]] std::uint8_t freeSlot() const;
    [[nodiscard]] Remote *find(std::uint32_t clientId);

    GatewayConfig _config;
    Send _send;
    Deliver _deliver;
    Drop _drop;
    protocol::ServerHandshake _handshake;
    protocol::RateLimiter _datagramLimiter;
    mutable std::mutex _mutex;
    NetCounters _totals;
    std::uint32_t _nextClientId = 1;
    std::unordered_map<std::string, std::unique_ptr<Remote>> _byAddress;
    std::unordered_map<std::uint32_t, Remote *> _byId;
};

} /* namespace rtype::server */

#endif /* !GATEWAY_HPP_ */
