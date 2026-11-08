/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ReceiveWindow.cpp
*/

#include "ReceiveWindow.hpp"

#include "SerialNumber.hpp"

namespace rtype::protocol {

namespace {

constexpr std::uint16_t WINDOW_SIZE = 32;

}

SeqVerdict ReceiveWindow::check(std::uint16_t seq) const noexcept
{
    if (!_started || isNewer(seq, _latest)) {
        return SeqVerdict::ACCEPT;
    }
    const std::uint16_t age = distance(_latest, seq);

    if (age == 0) {
        return SeqVerdict::DUPLICATE;
    }
    if (age > WINDOW_SIZE) {
        return SeqVerdict::TOO_OLD;
    }
    const bool seen = (_seen >> (age - 1U)) & 1U;

    return seen ? SeqVerdict::DUPLICATE : SeqVerdict::ACCEPT;
}

void ReceiveWindow::markSeen(std::uint16_t age) noexcept
{
    if (age >= 1 && age <= WINDOW_SIZE) {
        _seen |= 1U << (age - 1U);
    }
}

void ReceiveWindow::advance(std::uint16_t seq) noexcept
{
    const std::uint16_t shift = distance(seq, _latest);

    _seen = shift < WINDOW_SIZE ? _seen << shift : 0;
    if (shift <= WINDOW_SIZE) {
        _seen |= 1U << (shift - 1U);
    }
    _latest = seq;
}

void ReceiveWindow::commit(std::uint16_t seq) noexcept
{
    if (!_started) {
        _started = true;
        _latest = seq;
    } else if (isNewer(seq, _latest)) {
        advance(seq);
    } else {
        markSeen(distance(_latest, seq));
    }
}

bool ReceiveWindow::hasReceived() const noexcept
{
    return _started;
}

std::uint16_t ReceiveWindow::latest() const noexcept
{
    return _latest;
}

std::uint32_t ReceiveWindow::seenMask() const noexcept
{
    return _seen;
}

} /* namespace rtype::protocol */
