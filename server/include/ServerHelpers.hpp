/*
** EPITECH PROJECT, 2026
** R-TYPE
** File description:
** ServerHelpers
*/

#ifndef SERVERHELPER_HPP_
#define SERVERHELPER_HPP_

#include <cstdint>

class ServerHelpers {
public:
    static std::uint16_t readUint16BigEndian(const char *data) noexcept;
    ~ServerHelpers() = default;

protected:
private:
};

#endif /* SERVERHELPER_HPP_ */
