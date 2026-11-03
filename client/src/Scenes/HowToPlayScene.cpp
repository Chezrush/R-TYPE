/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** HowToPlayScene.cpp
*/

#include "Client/Scenes/HowToPlayScene.hpp"

#include <memory>
#include <utility>

#include "Engine/Ecs/Components/UIButton.hpp"
#include "Engine/Ecs/Components/UIWidgetEvents.hpp"

namespace client::scenes {

namespace {

using engine::core::Vec2;
using engine::ecs::components::UIButton;
using engine::ecs::components::UIButtonClickedEvent;
using engine::ecs::components::UICancelEvent;
using settings::GameAction;

constexpr engine::ecs::components::UIColor BACKDROP_COLOR{5, 8, 24, 245};
constexpr unsigned int TITLE_TEXT_SIZE = 40;
constexpr unsigned int LINE_TEXT_SIZE = 18;
constexpr Vec2 TITLE_OFFSET{0.0f, -212.0f};
constexpr Vec2 TITLE_SIZE{600.0f, 44.0f};
constexpr Vec2 LINE_SIZE{840.0f, 24.0f};
constexpr float FIRST_LINE_Y = -170.0f;
constexpr float LINE_SPACING = 27.0f;
constexpr Vec2 BACK_OFFSET{0.0f, 200.0f};
constexpr Vec2 BACK_SIZE{260.0f, 48.0f};

std::string actionLine(const settings::KeyBindings &bindings, GameAction action)
{
    return settings::actionName(action) + ": " +
           settings::bindingText(bindings, action);
}

std::string beamLine(const settings::KeyBindings &bindings)
{
    return bindings.beamMode == settings::BeamChargeMode::HOLD
               ? "Hold shoot to charge your beam."
               : "Press shoot to start or stop charging your beam.";
}

std::string fireLine(const settings::KeyBindings &bindings)
{
    return bindings.autoFire ? "Auto-fire is on: you keep shooting by yourself."
                             : "Auto-fire is off: press shoot for each shot.";
}

} // namespace

std::vector<std::string> howToPlayLines(const settings::KeyBindings &bindings)
{
    std::vector<std::string> lines{
        "Destroy the enemies and stay alive.",
        "Up to 4 players share the same game.",
        "The bottom bar shows the players, lives, stage, beam and score.",
        "If your ship is destroyed, it comes back after a short wait.",
        beamLine(bindings),
        fireLine(bindings),
    };

    lines.reserve(lines.size() + settings::GAME_ACTIONS.size());
    for (const GameAction action : settings::GAME_ACTIONS)
        lines.push_back(actionLine(bindings, action));
    return lines;
}

HowToPlayScene::HowToPlayScene(engine::ecs::UISceneContext context,
                               const settings::KeyBindings &bindings)
    : UIOverlay(std::move(context)),
      _bindings(bindings)
{}

std::optional<engine::ecs::Entity> HowToPlayScene::backButton() const noexcept
{
    return _back;
}

void HowToPlayScene::activate()
{
    float y = FIRST_LINE_Y;

    addBackdrop(BACKDROP_COLOR);
    addLabel("How to play", TITLE_OFFSET, TITLE_SIZE, TITLE_TEXT_SIZE);
    for (const std::string &line : howToPlayLines(_bindings)) {
        addLabel(line, {0.0f, y}, LINE_SIZE, LINE_TEXT_SIZE);
        y += LINE_SPACING;
    }
    const engine::ecs::Entity back =
        addButton("Back", "how-to-play.back", BACK_OFFSET, BACK_SIZE);

    _back = back;
    widget<UIButton>(back).isFocused = true;
    listen<UIButtonClickedEvent>([this](const UIButtonClickedEvent &) {
        bus().publish(engine::core::OverlayCloseRequested{});
    });
    listen<UICancelEvent>([this](const UICancelEvent &) {
        bus().publish(engine::core::OverlayCloseRequested{});
    });
}

void addHowToPlayRoute(SceneRouter &router,
                       const engine::ecs::UISceneContext &context,
                       const settings::KeyBindings &bindings)
{
    router.setRoute(SceneRoute::HOW_TO_PLAY, [context, &bindings] {
        return std::make_unique<HowToPlayScene>(context, bindings);
    });
}

} // namespace client::scenes
