/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** InGameMenuOverlay.hpp
*/

#ifndef INGAMEMENUOVERLAY_HPP_
#define INGAMEMENUOVERLAY_HPP_

#include <string>
#include <vector>

#include "Engine/Ecs/Components/UIButton.hpp"
#include "Engine/Ecs/UIOverlay.hpp"

namespace client::scenes {

/**
 * @brief Menu above the game, which keeps running behind it.
 *
 * Resume (or Escape) publishes ResumeRequested. Settings and Controls open
 * their screen above the menu. Disconnect publishes DisconnectRequested, then
 * asks for the main menu.
 */
class InGameMenuOverlay : public engine::ecs::UIOverlay {
public:
    explicit InGameMenuOverlay(engine::ecs::UISceneContext context);

    /** @brief The widget of a button, by its label ("Resume", ...). */
    [[nodiscard]] engine::ecs::Entity button(const std::string &label) const;

protected:
    void activate() override;

private:
    void onButton(const engine::ecs::components::UIButtonClickedEvent &event);

    std::vector<engine::ecs::Entity> _buttons;
};

} // namespace client::scenes

#endif /* !INGAMEMENUOVERLAY_HPP_ */
