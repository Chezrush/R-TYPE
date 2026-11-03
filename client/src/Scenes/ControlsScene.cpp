/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ControlsScene.cpp
*/

#include "Client/Scenes/ControlsScene.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <utility>

#include "Engine/Ecs/Components/UIDropdown.hpp"
#include "Engine/Ecs/Components/UIPanel.hpp"
#include "Engine/Ecs/Components/UIText.hpp"
#include "Engine/Ecs/Components/UIToggle.hpp"

namespace client::scenes {

namespace {

using engine::core::Vec2;
using engine::ecs::Entity;
using engine::ecs::components::UIAnchor;
using engine::ecs::components::UIButton;
using engine::ecs::components::UIButtonClickedEvent;
using engine::ecs::components::UICancelEvent;
using engine::ecs::components::UIColor;
using engine::ecs::components::UIDropdown;
using engine::ecs::components::UIPanel;
using engine::ecs::components::UISelectionChangedEvent;
using engine::ecs::components::UIText;
using engine::ecs::components::UITextAlign;
using engine::ecs::components::UIToggle;
using engine::ecs::components::UITransform;
using engine::ecs::components::UIValueChangedEvent;
using settings::GAME_ACTIONS;
using settings::GameAction;

constexpr std::size_t AUTO_FIRE_PART = 0;
constexpr std::size_t BEAM_PART = 1;
constexpr std::size_t WARNING_PART = 2;
constexpr std::size_t RESET_PART = 3;
constexpr std::size_t BACK_PART = 4;
constexpr std::size_t CAPTURE_PANEL_PART = 5;
constexpr std::size_t CAPTURE_LABEL_PART = 6;

constexpr const char *ACTION_ROW = "controls.action";
constexpr const char *AUTO_FIRE_ACTION = "controls.auto-fire";
constexpr const char *BEAM_ACTION = "controls.beam";
constexpr const char *RESET_ACTION = "controls.reset";
constexpr const char *BACK_ACTION = "controls.back";

constexpr UIColor BACKDROP_COLOR{5, 8, 24, 245};
constexpr UIColor WARNING_COLOR{255, 190, 80, 255};
constexpr UIColor CAPTURE_COLOR{0, 0, 0, 225};
constexpr UIColor HIDDEN{0, 0, 0, 0};
constexpr unsigned int TITLE_TEXT_SIZE = 40;
constexpr unsigned int ROW_TEXT_SIZE = 18;
constexpr unsigned int CAPTURE_TEXT_SIZE = 26;
constexpr int CAPTURE_Z_INDEX = 50;
constexpr Vec2 TITLE_OFFSET{0.0f, -212.0f};
constexpr Vec2 TITLE_SIZE{600.0f, 44.0f};
constexpr float FIRST_ROW_Y = -165.0f;
constexpr float ROW_SPACING = 40.0f;
constexpr Vec2 ROW_SIZE{640.0f, 34.0f};
constexpr float OPTIONS_Y = 80.0f;
constexpr Vec2 OPTION_LABEL_SIZE{150.0f, 32.0f};
constexpr Vec2 TOGGLE_SIZE{60.0f, 32.0f};
constexpr Vec2 BEAM_SIZE{150.0f, 32.0f};
constexpr float AUTO_FIRE_LABEL_X = -245.0f;
constexpr float AUTO_FIRE_X = -130.0f;
constexpr float BEAM_LABEL_X = 80.0f;
constexpr float BEAM_X = 245.0f;
constexpr Vec2 WARNING_OFFSET{0.0f, 125.0f};
constexpr Vec2 WARNING_SIZE{800.0f, 28.0f};
constexpr Vec2 BUTTON_SIZE{260.0f, 48.0f};
constexpr float BUTTON_X = 140.0f;
constexpr float BUTTON_Y = 178.0f;

std::string rowText(const settings::KeyBindings &bindings, GameAction action)
{
    return settings::actionName(action) + ":  " +
           settings::bindingText(bindings, action);
}

std::string captureText(GameAction action)
{
    const std::string text =
        "Press a key or a button for " + settings::actionName(action);

    return action == GameAction::OPEN_MENU ? text : text + " (Escape cancels)";
}

} // namespace

ControlsScene::ControlsScene(engine::ecs::UISceneContext context,
                             settings::SettingsStore &store)
    : UIOverlay(std::move(context)),
      _store(store)
{}

void ControlsScene::onExit()
{
    static_cast<void>(_store.save());
    UIOverlay::onExit();
}

void ControlsScene::handleInput()
{
    if (_capturing.has_value() || _discardNextInput) {
        interaction().discardInput();
        _discardNextInput = false;
        return;
    }
    UIOverlay::handleInput();
}

Entity ControlsScene::actionButton(GameAction action) const
{
    return _rows.at(static_cast<std::size_t>(action));
}

Entity ControlsScene::autoFireToggle() const
{
    return _parts.at(AUTO_FIRE_PART);
}

Entity ControlsScene::beamDropdown() const
{
    return _parts.at(BEAM_PART);
}

Entity ControlsScene::resetButton() const
{
    return _parts.at(RESET_PART);
}

Entity ControlsScene::backButton() const
{
    return _parts.at(BACK_PART);
}

Entity ControlsScene::warningLabel() const
{
    return _parts.at(WARNING_PART);
}

Entity ControlsScene::captureLabel() const
{
    return _parts.at(CAPTURE_LABEL_PART);
}

bool ControlsScene::isCapturing() const noexcept
{
    return _capturing.has_value();
}

void ControlsScene::activate()
{
    _rows.clear();
    _parts.clear();
    _capturing.reset();
    addBackdrop(BACKDROP_COLOR);
    addLabel("Controls", TITLE_OFFSET, TITLE_SIZE, TITLE_TEXT_SIZE);
    buildActions();
    buildOptions();
    buildButtons();
    buildCaptureModal();
    syncWidgets();
    widget<UIButton>(_rows.front()).isFocused = true;
    listenToWidgets();
    listenToDevices();
}

void ControlsScene::buildActions()
{
    float y = FIRST_ROW_Y;

    for (std::size_t i = 0; i < GAME_ACTIONS.size(); ++i) {
        _rows.push_back(addButton("", ACTION_ROW, {0.0f, y}, ROW_SIZE));
        widget<UIText>(_rows.back()).characterSize = ROW_TEXT_SIZE;
        y += ROW_SPACING;
    }
}

void ControlsScene::buildOptions()
{
    const Entity toggle =
        spawn(UIAnchor::CENTER, {AUTO_FIRE_X, OPTIONS_Y}, TOGGLE_SIZE);
    const Entity beam = spawn(UIAnchor::CENTER, {BEAM_X, OPTIONS_Y}, BEAM_SIZE);
    UIToggle autoFire;
    UIDropdown mode;

    autoFire.action = AUTO_FIRE_ACTION;
    mode.action = BEAM_ACTION;
    mode.options = {"Hold", "Toggle"};
    mode.style.fontId = fontId();
    registry().addComponent(toggle, std::move(autoFire));
    registry().addComponent(beam, std::move(mode));
    addLabel("Auto-fire", {AUTO_FIRE_LABEL_X, OPTIONS_Y}, OPTION_LABEL_SIZE,
             ROW_TEXT_SIZE);
    addLabel("Beam charge", {BEAM_LABEL_X, OPTIONS_Y}, OPTION_LABEL_SIZE,
             ROW_TEXT_SIZE);
    _parts.push_back(toggle);
    _parts.push_back(beam);
}

void ControlsScene::buildButtons()
{
    const Entity warning =
        addLabel("", WARNING_OFFSET, WARNING_SIZE, ROW_TEXT_SIZE);

    widget<UIText>(warning).color = WARNING_COLOR;
    widget<UIText>(warning).colorRole = engine::ecs::UIColorRole::WARNING;
    _parts.push_back(warning);
    _parts.push_back(addButton("Reset to defaults", RESET_ACTION,
                               {-BUTTON_X, BUTTON_Y}, BUTTON_SIZE));
    _parts.push_back(
        addButton("Back", BACK_ACTION, {BUTTON_X, BUTTON_Y}, BUTTON_SIZE));
}

void ControlsScene::buildCaptureModal()
{
    const Entity panel = addBackdrop(HIDDEN);
    const Entity label =
        addLabel("", {0.0f, 0.0f}, WARNING_SIZE, CAPTURE_TEXT_SIZE);

    widget<UITransform>(panel).zIndex += CAPTURE_Z_INDEX + 1;
    widget<UITransform>(label).zIndex += CAPTURE_Z_INDEX + 1;
    _parts.push_back(panel);
    _parts.push_back(label);
}

void ControlsScene::listenToWidgets()
{
    listen<UIButtonClickedEvent>(
        [this](const UIButtonClickedEvent &event) { onButton(event); });
    listen<UICancelEvent>([this](const UICancelEvent &) {
        bus().publish(engine::core::OverlayCloseRequested{});
    });
    listen<UIValueChangedEvent>([this](const UIValueChangedEvent &event) {
        if (event.action == AUTO_FIRE_ACTION) {
            _store.settings().bindings.autoFire = event.value > 0.5f;
            changed();
        }
    });
    listen<UISelectionChangedEvent>(
        [this](const UISelectionChangedEvent &event) {
            if (event.action == BEAM_ACTION) {
                _store.settings().bindings.beamMode =
                    static_cast<settings::BeamChargeMode>(event.index);
                changed();
            }
        });
}

void ControlsScene::listenToDevices()
{
    listen<events::RawKeyPressed>(
        [this](const events::RawKeyPressed &event) { onKey(event.key); });
    listen<events::RawGamepadButtonPressed>(
        [this](const events::RawGamepadButtonPressed &event) {
            onGamepadButton(event.button);
        });
}

void ControlsScene::onButton(const UIButtonClickedEvent &event)
{
    if (event.action == RESET_ACTION) {
        reset();
    } else if (event.action == BACK_ACTION) {
        bus().publish(engine::core::OverlayCloseRequested{});
    } else if (event.action == ACTION_ROW) {
        for (std::size_t i = 0; i < _rows.size(); ++i) {
            if (_rows.at(i).index() == event.entityIndex)
                startCapture(GAME_ACTIONS.at(i));
        }
    }
}

void ControlsScene::onKey(settings::ScanCode key)
{
    if (!_capturing.has_value())
        return;
    const GameAction action = *_capturing;

    stopCapture();
    if (key == settings::KEY_ESCAPE && action != GameAction::OPEN_MENU)
        return;
    settings::setKey(_store.settings().bindings, action, key);
    changed();
}

void ControlsScene::onGamepadButton(settings::GamepadButton button)
{
    if (!_capturing.has_value())
        return;
    const GameAction action = *_capturing;

    stopCapture();
    settings::setButton(_store.settings().bindings, action, button);
    changed();
}

void ControlsScene::startCapture(GameAction action)
{
    _capturing = action;
    widget<UIPanel>(_parts.at(CAPTURE_PANEL_PART)).color = CAPTURE_COLOR;
    widget<UIText>(captureLabel()).text = captureText(action);
}

void ControlsScene::stopCapture()
{
    _capturing.reset();
    _discardNextInput = true;
    widget<UIPanel>(_parts.at(CAPTURE_PANEL_PART)).color = HIDDEN;
    widget<UIText>(captureLabel()).text.clear();
}

void ControlsScene::reset()
{
    _store.settings().bindings = settings::KeyBindings{};
    changed();
}

void ControlsScene::changed()
{
    syncWidgets();
    bus().publish(events::BindingsChanged{_store.settings().bindings});
}

void ControlsScene::syncWidgets()
{
    const settings::KeyBindings &bindings = _store.settings().bindings;

    for (std::size_t i = 0; i < _rows.size(); ++i)
        widget<UIText>(_rows.at(i)).text =
            rowText(bindings, GAME_ACTIONS.at(i));
    widget<UIToggle>(autoFireToggle()).checked = bindings.autoFire;
    widget<UIDropdown>(beamDropdown()).selected =
        static_cast<std::size_t>(bindings.beamMode);
    widget<UIText>(warningLabel()).text = settings::conflictWarning(bindings);
}

void addControlsRoute(SceneRouter &router,
                      const engine::ecs::UISceneContext &context,
                      settings::SettingsStore &store)
{
    router.setRoute(SceneRoute::CONTROLS, [context, &store] {
        return std::make_unique<ControlsScene>(context, store);
    });
}

} // namespace client::scenes
