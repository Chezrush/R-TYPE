/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ClientHandshake.hpp
*/

#ifndef CLIENTHANDSHAKE_HPP_
#define CLIENTHANDSHAKE_HPP_

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

#include "Messages.hpp"
#include "Timing.hpp"

namespace rtype::protocol {

/**
 * @brief Client side of the connection handshake.
 *
 * It sends hello every 500 ms (with the cookie once the server gave one),
 * gives up after 10 s, and accepts the first welcome. It does no I/O: the
 * caller sends what poll() returns and passes every datagram received from
 * the server to onDatagram().
 */
class ClientHandshake {
public:
    enum class State {
        CONNECTING,
        CONNECTED,
        REJECTED,
        FAILED
    };

    static constexpr Duration RETRY_DELAY = std::chrono::milliseconds(500);
    static constexpr Duration GIVE_UP_DELAY = std::chrono::seconds(10);

    ClientHandshake(std::string name, TimePoint start);

    /**
     * @brief Returns the datagram to send now, if one is due.
     */
    [[nodiscard]] std::optional<std::string> poll(TimePoint now);

    void onDatagram(std::string_view datagram);

    [[nodiscard]] State state() const noexcept;
    [[nodiscard]] const std::optional<Welcome> &welcome() const noexcept;
    [[nodiscard]] std::uint32_t session() const noexcept;

    /**
     * @brief Reason of the reject, valid when the state is REJECTED.
     */
    [[nodiscard]] std::uint8_t rejectReason() const noexcept;

    /**
     * @brief Protocol version of a server that rejected ours, 0 if unknown.
     */
    [[nodiscard]] std::uint8_t serverVersion() const noexcept;

private:
    void apply(const Message &message, std::uint32_t session);
    void handleRecords(std::string_view datagram, std::uint32_t session);
    [[nodiscard]] std::string buildHello();

    std::string _name;
    TimePoint _start;
    TimePoint _lastSent;
    bool _sentOnce = false;
    bool _sendNow = false;
    State _state = State::CONNECTING;
    std::uint64_t _cookie = 0;
    std::uint16_t _seq = 0;
    std::uint32_t _session = 0;
    std::uint8_t _rejectReason = 0;
    std::uint8_t _serverVersion = 0;
    std::optional<Welcome> _welcome;
};

} /* namespace rtype::protocol */

#endif /* !CLIENTHANDSHAKE_HPP_ */
