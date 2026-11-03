/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** MainMenuScene.hpp
*/

#ifndef MAINMENUSCENE_HPP_
#define MAINMENUSCENE_HPP_

#include <string>
#include <vector>

#include "Client/Scenes/Starfield.hpp"
#include "Engine/Ecs/UIScene.hpp"

namespace client::scenes {

/**
 * @brief First screen: logo, scrolling starfield and the main buttons.
 *
 * Play publishes a NavigationRequested, Quit a QuitRequested, and the others
 * an OverlayRequested, so that going back returns to this menu. The first
 * one has the focus, so the menu works without a mouse from the start.
 */
class MainMenuScene : public engine::ecs::UIScene {
public:
    explicit MainMenuScene(engine::ecs::UISceneContext context);

    void update(float dt) override;

    /** @brief The widget of a button, by its label ("Play", "Quit", ...). */
    [[nodiscard]] engine::ecs::Entity button(const std::string &label) const;

    [[nodiscard]] const Starfield &starfield() const noexcept;

protected:
    void activate() override;

private:
    void buildStars();
    void buildButtons();
    void placeStars();
    void onButton(const engine::ecs::UIButtonClickedEvent &event);

    Starfield _starfield;
    std::vector<engine::ecs::Entity> _stars;
    std::vector<engine::ecs::Entity> _buttons;
};

} // namespace client::scenes

#endif /* !MAINMENUSCENE_HPP_ */
