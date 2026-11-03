/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ChatModel.hpp
*/

#ifndef CHATMODEL_HPP_
#define CHATMODEL_HPP_

#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <optional>
#include <string>
#include <vector>

#include "Client/Events/LobbyEvents.hpp"

namespace client::lobby {

/** @brief Longest chat message. */
inline constexpr std::size_t MAX_CHAT_LENGTH = 120;

/** @brief What a line typed in the chat asks for. */
struct ChatCommand {
    enum class Kind : std::uint8_t {
        SEND,
        MUTE,
        UNMUTE,
        INVALID,
        NOTHING
    };

    Kind kind = Kind::NOTHING;
    std::string text;
    std::uint8_t player = 0;
};

/**
 * @brief Chat log and muted players, with no UI.
 *
 * "/mute 2" and "/unmute 2" (the player numbers of the HUD) are handled
 * locally and never sent. A muted player's lines are hidden, and come back on
 * unmute.
 */
class ChatModel {
public:
    static constexpr std::size_t MAX_LINES = 100;

    /** @brief What the typed line asks for. */
    [[nodiscard]] static ChatCommand parse(const std::string &line);

    /** @brief "[03:25]" for a time in seconds. */
    [[nodiscard]] static std::string timestamp(double seconds);

    void add(const events::ChatMessageReceived &message, double seconds);
    void addNotice(const std::string &text, double seconds);
    void setMuted(std::uint8_t player, bool muted) noexcept;
    [[nodiscard]] bool isMuted(std::uint8_t player) const noexcept;

    /** @brief The visible lines, oldest first. */
    [[nodiscard]] std::vector<std::string> lines() const;

private:
    struct Line {
        std::uint8_t player;
        std::string text;
    };

    std::deque<Line> _lines;
    std::array<bool, events::LOBBY_SLOTS> _muted{};
};

} // namespace client::lobby

#endif /* !CHATMODEL_HPP_ */
