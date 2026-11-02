/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ConnectForm.hpp
*/

#ifndef CONNECTFORM_HPP_
#define CONNECTFORM_HPP_

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

#include "Client/Events/ConnectionEvents.hpp"

namespace client::scenes {

/** @brief Longest nickname of the protocol `hello`, in bytes. */
inline constexpr std::size_t MAX_NICKNAME_BYTES = 16;

/** @brief Content of the connection form. */
struct ConnectForm {
    std::string host = "127.0.0.1";
    std::string port = "4242";
    std::string nickname;

    friend bool operator==(const ConnectForm &, const ConnectForm &) = default;
};

/** @brief IPv4 address or host name. */
[[nodiscard]] bool isValidHost(const std::string &host);

/** @brief The port, if the text is a number from 1 to 65535. */
[[nodiscard]] std::optional<std::uint16_t> parsePort(const std::string &port);

/** @brief 1 to 16 bytes of printable ASCII, as the protocol requires. */
[[nodiscard]] bool isValidNickname(const std::string &nickname);

/** @brief What is wrong with the form, or nothing if it can be sent. */
[[nodiscard]] std::optional<std::string>
validationError(const ConnectForm &form);

/** @brief "Connecting to 127.0.0.1:4242..." */
[[nodiscard]] std::string connectingMessage(const std::string &host,
                                            std::uint16_t port);

/** @brief Readable message for a failure, unknown reasons included. */
[[nodiscard]] std::string failureMessage(events::ConnectionFailure reason);

} // namespace client::scenes

#endif /* !CONNECTFORM_HPP_ */
