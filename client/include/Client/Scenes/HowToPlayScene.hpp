/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** HowToPlayScene.hpp
*/

#ifndef HOWTOPLAYSCENE_HPP_
#define HOWTOPLAYSCENE_HPP_

#include <optional>
#include <string>
#include <vector>

#include "Client/Scenes/SceneRoute.hpp"
#include "Client/Settings/KeyBindings.hpp"
#include "Engine/Ecs/UIOverlay.hpp"

namespace client::scenes {

/** @brief The goal, the HUD and the player's real controls, in short sentences.
 */
[[nodiscard]] std::vector<std::string>
howToPlayLines(const settings::KeyBindings &bindings);

/** @brief Explains the game, above the main menu or the in-game menu. */
class HowToPlayScene : public engine::ecs::UIOverlay {
public:
    HowToPlayScene(engine::ecs::UISceneContext context,
                   const settings::KeyBindings &bindings);

    [[nodiscard]] std::optional<engine::ecs::Entity>
    backButton() const noexcept;

protected:
    void activate() override;

private:
    const settings::KeyBindings &_bindings;
    std::optional<engine::ecs::Entity> _back;
};

/** @brief Gives the router the how to play screen. */
void addHowToPlayRoute(SceneRouter &router,
                       const engine::ecs::UISceneContext &context,
                       const settings::KeyBindings &bindings);

} // namespace client::scenes

#endif /* !HOWTOPLAYSCENE_HPP_ */
