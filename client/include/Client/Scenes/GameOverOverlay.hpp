/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameOverOverlay.hpp
*/

#ifndef GAMEOVEROVERLAY_HPP_
#define GAMEOVEROVERLAY_HPP_

#include <cstdint>
#include <optional>

#include "Engine/Ecs/Components/UIButton.hpp"
#include "Engine/Ecs/UIOverlay.hpp"

namespace client::scenes {

/**
 * @brief End of the game: final score and a way back to the menu.
 *
 * "Back to menu" publishes DisconnectRequested, since the player leaves the
 * server, then asks for the main menu.
 */
class GameOverOverlay : public engine::ecs::UIOverlay {
public:
    GameOverOverlay(engine::ecs::UISceneContext context, std::uint32_t score);

    [[nodiscard]] std::optional<engine::ecs::Entity>
    scoreLabel() const noexcept;
    [[nodiscard]] std::optional<engine::ecs::Entity>
    backButton() const noexcept;

protected:
    void activate() override;

private:
    std::uint32_t _score;
    std::optional<engine::ecs::Entity> _scoreLabel;
    std::optional<engine::ecs::Entity> _back;
};

} // namespace client::scenes

#endif /* !GAMEOVEROVERLAY_HPP_ */
