/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** LocalAddress.cpp
*/

#include "LocalAddress.hpp"

#include <algorithm>
#include <array>
#include <asio.hpp>
#include <cstring>
#include <memory>
#include <system_error>

#ifndef _WIN32
#    include <arpa/inet.h>
#    include <ifaddrs.h>
#    include <net/if.h>
#    include <netinet/in.h>
#    include <sys/socket.h>
#endif

namespace rtype::server {

namespace {

using Udp = asio::ip::udp;

void addAddress(std::vector<std::string> &out, const std::string &text)
{
    if (isReachableAddress(text) &&
        std::find(out.begin(), out.end(), text) == out.end()) {
        out.push_back(text);
    }
}

std::string outgoingAddress()
{
    asio::io_context context;
    Udp::socket socket(context);
    std::error_code error;

    error = socket.open(Udp::v4(), error);
    if (error) {
        return {};
    }
    error = socket.connect(
        Udp::endpoint(asio::ip::make_address("192.0.2.1"), 9), error);
    if (error) {
        return {};
    }
    return socket.local_endpoint(error).address().to_string();
}

void addHostAddresses(std::vector<std::string> &out)
{
    asio::io_context context;
    Udp::resolver resolver(context);
    std::error_code error;
    const auto found = resolver.resolve(asio::ip::host_name(error), "", error);

    if (error) {
        return;
    }
    for (const auto &entry : found) {
        addAddress(out, entry.endpoint().address().to_string());
    }
}

#ifndef _WIN32
bool isUsableInterface(const ifaddrs &item)
{
    const unsigned flags = item.ifa_flags;

    return item.ifa_addr != nullptr && item.ifa_addr->sa_family == AF_INET &&
           (flags & static_cast<unsigned>(IFF_UP)) != 0U &&
           (flags & static_cast<unsigned>(IFF_LOOPBACK)) == 0U;
}

std::string textOf(const ifaddrs &item)
{
    sockaddr_in address{};
    std::array<char, INET_ADDRSTRLEN> text{};

    std::memcpy(&address, item.ifa_addr, sizeof(address));
    if (inet_ntop(AF_INET, &address.sin_addr, text.data(), text.size()) ==
        nullptr) {
        return {};
    }
    return text.data();
}

void addInterfaceAddresses(std::vector<std::string> &out)
{
    ifaddrs *list = nullptr;

    if (getifaddrs(&list) != 0) {
        return;
    }
    const std::unique_ptr<ifaddrs, decltype(&freeifaddrs)> guard(list,
                                                                 freeifaddrs);

    for (const ifaddrs *item = list; item != nullptr; item = item->ifa_next) {
        if (isUsableInterface(*item)) {
            addAddress(out, textOf(*item));
        }
    }
}
#else
void addInterfaceAddresses(std::vector<std::string> &) {}
#endif

} // namespace

bool isReachableAddress(const std::string &text)
{
    std::error_code error;
    const auto address = asio::ip::make_address(text, error);

    return !error && address.is_v4() && !address.is_loopback() &&
           !address.is_unspecified();
}

std::vector<std::string> localAddresses()
{
    std::vector<std::string> out;

    addAddress(out, outgoingAddress());
    addInterfaceAddresses(out);
    addHostAddresses(out);
    return out;
}

} /* namespace rtype::server */
