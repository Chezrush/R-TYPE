/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** OverlayEvents.hpp
*/

#ifndef OVERLAYEVENTS_HPP_
#define OVERLAYEVENTS_HPP_

namespace client::events {

/** @brief The in-game menu opened: stop sending moves, duck the music... */
struct InGameMenuOpened {};

/** @brief The in-game menu closed. */
struct InGameMenuClosed {};

/** @brief The player chose "Resume" or pressed Escape in the in-game menu. */
struct ResumeRequested {};

/** @brief The player leaves the game: the network layer says goodbye. */
struct DisconnectRequested {};

/** @brief Typed keys go to a text field: the input code stops moving the ship.
 */
struct TextEntryStarted {};

/** @brief Typed keys can drive the game again. */
struct TextEntryEnded {};

} // namespace client::events

#endif /* !OVERLAYEVENTS_HPP_ */
