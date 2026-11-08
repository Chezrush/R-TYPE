/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** NoticeOverlay.hpp
*/

#ifndef NOTICEOVERLAY_HPP_
#define NOTICEOVERLAY_HPP_

#include <optional>
#include <string>

#include "Engine/Ecs/UIOverlay.hpp"

namespace client::scenes {

/**
 * @brief Modal message above the main menu, closed by OK or Escape.
 *
 * Used to tell why the player came back to the menu (connection lost...).
 */
class NoticeOverlay : public engine::ecs::UIOverlay {
public:
    NoticeOverlay(engine::ecs::UISceneContext context, std::string message);

    [[nodiscard]] const std::string &message() const noexcept;
    [[nodiscard]] engine::ecs::Entity okButton() const;

protected:
    void activate() override;

private:
    std::string _message;
    std::optional<engine::ecs::Entity> _ok;
};

} // namespace client::scenes

#endif /* !NOTICEOVERLAY_HPP_ */
