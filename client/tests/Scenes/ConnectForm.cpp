/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ConnectForm.cpp
*/

#include "Client/Scenes/ConnectForm.hpp"

#include <catch2/catch_test_macros.hpp>
#include <string>

using client::events::ConnectionFailure;
using client::scenes::ConnectForm;
using client::scenes::connectingMessage;
using client::scenes::failureMessage;
using client::scenes::isValidHost;
using client::scenes::isValidNickname;
using client::scenes::parsePort;
using client::scenes::validationError;

TEST_CASE("addresses are IPv4 or host names", "[client][scenes][form]")
{
    REQUIRE(isValidHost("127.0.0.1"));
    REQUIRE(isValidHost("255.255.255.255"));
    REQUIRE(isValidHost("localhost"));
    REQUIRE(isValidHost("r-type.example.com"));
    REQUIRE_FALSE(isValidHost(""));
    REQUIRE_FALSE(isValidHost("256.0.0.1"));
    REQUIRE_FALSE(isValidHost("127.0.0"));
    REQUIRE_FALSE(isValidHost("1.2.3.4.5"));
    REQUIRE_FALSE(isValidHost("1..2.3"));
    REQUIRE_FALSE(isValidHost("-bad.com"));
    REQUIRE_FALSE(isValidHost("bad host"));
    REQUIRE_FALSE(isValidHost(std::string(254, 'a')));
}

TEST_CASE("ports go from 1 to 65535", "[client][scenes][form]")
{
    REQUIRE(parsePort("1") == 1);
    REQUIRE(parsePort("4242") == 4242);
    REQUIRE(parsePort("65535") == 65535);
    REQUIRE_FALSE(parsePort("").has_value());
    REQUIRE_FALSE(parsePort("0").has_value());
    REQUIRE_FALSE(parsePort("65536").has_value());
    REQUIRE_FALSE(parsePort("42a").has_value());
    REQUIRE_FALSE(parsePort("-1").has_value());
    REQUIRE_FALSE(parsePort("99999999999999").has_value());
}

TEST_CASE("nicknames are 1 to 16 bytes of printable ASCII",
          "[client][scenes][form]")
{
    REQUIRE(isValidNickname("B"));
    REQUIRE(isValidNickname("Bob the 2nd"));
    REQUIRE(isValidNickname(std::string(16, 'x')));
    REQUIRE_FALSE(isValidNickname(""));
    REQUIRE_FALSE(isValidNickname(std::string(17, 'x')));
    REQUIRE_FALSE(isValidNickname("Ren\xC3\xA9"));
    REQUIRE_FALSE(isValidNickname("a\tb"));
}

TEST_CASE("the form names its first error", "[client][scenes][form]")
{
    ConnectForm form{"127.0.0.1", "4242", "Bob"};
    REQUIRE_FALSE(validationError(form).has_value());

    form.nickname.clear();
    REQUIRE(validationError(form)->find("nickname") != std::string::npos);
    form.port = "0";
    REQUIRE(validationError(form)->find("port") != std::string::npos);
    form.host = "1.2.3";
    REQUIRE(validationError(form)->find("address") != std::string::npos);
}

TEST_CASE("every failure has its own readable message",
          "[client][scenes][form]")
{
    const auto server = failureMessage(ConnectionFailure::SERVER_FULL);

    REQUIRE(server.find("full") != std::string::npos);
    for (const ConnectionFailure other :
         {ConnectionFailure::INVALID_NAME, ConnectionFailure::VERSION,
          ConnectionFailure::RATE_LIMITED, ConnectionFailure::SHUTTING_DOWN,
          ConnectionFailure::TIMEOUT, ConnectionFailure::INVALID_ADDRESS,
          static_cast<ConnectionFailure>(99)}) {
        REQUIRE_FALSE(failureMessage(other).empty());
        REQUIRE(failureMessage(other) != server);
    }
}

TEST_CASE("the progress message names the server", "[client][scenes][form]")
{
    REQUIRE(connectingMessage("127.0.0.1", 4242) ==
            "Connecting to 127.0.0.1:4242...");
}
