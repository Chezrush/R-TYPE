/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** RespawnOverlay.hpp
*/

#ifndef RESPAWNOVERLAY_HPP_
#define RESPAWNOVERLAY_HPP_

#include <optional>

#include "Client/Scenes/GameOverlays.hpp"
#include "Engine/Ecs/UIOverlay.hpp"

namespace client::scenes {

/** @brief Countdown shown while our ship is destroyed. */
class RespawnOverlay : public engine::ecs::UIOverlay {
public:
    RespawnOverlay(engine::ecs::UISceneContext context,
                   const GameOverlays &overlays);

    void update(float dt) override;

    [[nodiscard]] std::optional<engine::ecs::Entity> label() const noexcept;

protected:
    void activate() override;

private:
    void syncLabel();

    const GameOverlays &_overlays;
    std::optional<engine::ecs::Entity> _label;
};

} // namespace client::scenes

#endif /* !RESPAWNOVERLAY_HPP_ */
