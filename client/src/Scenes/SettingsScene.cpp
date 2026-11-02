/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** SettingsScene.cpp
*/

#include "Client/Scenes/SettingsScene.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>
#include <memory>
#include <string>
#include <utility>

#include "Engine/Ecs/Components/UIDropdown.hpp"
#include "Engine/Ecs/Components/UISlider.hpp"
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
using engine::ecs::components::UISelectionChangedEvent;
using engine::ecs::components::UISlider;
using engine::ecs::components::UIText;
using engine::ecs::components::UITextAlign;
using engine::ecs::components::UIToggle;
using engine::ecs::components::UIValueChangedEvent;

constexpr const char *MASTER_ACTION = "settings.master";
constexpr const char *MUSIC_ACTION = "settings.music";
constexpr const char *SFX_ACTION = "settings.sfx";
constexpr const char *MUTE_ACTION = "settings.mute";
constexpr const char *FULLSCREEN_ACTION = "settings.fullscreen";
constexpr const char *RESOLUTION_ACTION = "settings.resolution";
constexpr const char *VSYNC_ACTION = "settings.vsync";
constexpr const char *UI_SCALE_ACTION = "settings.ui-scale";
constexpr const char *RESET_ACTION = "settings.reset";
constexpr const char *BACK_ACTION = "settings.back";

constexpr UIColor BACKDROP_COLOR{5, 8, 24, 245};
constexpr unsigned int TITLE_TEXT_SIZE = 40;
constexpr unsigned int HEADER_TEXT_SIZE = 24;
constexpr unsigned int ROW_TEXT_SIZE = 18;
constexpr Vec2 TITLE_OFFSET{0.0f, -212.0f};
constexpr Vec2 TITLE_SIZE{600.0f, 50.0f};
constexpr Vec2 HEADER_SIZE{400.0f, 30.0f};
constexpr Vec2 LABEL_SIZE{190.0f, 36.0f};
constexpr Vec2 CONTROL_SIZE{190.0f, 32.0f};
constexpr Vec2 BUTTON_SIZE{260.0f, 48.0f};
constexpr float LEFT_COLUMN = -215.0f;
constexpr float RIGHT_COLUMN = 215.0f;
constexpr float CELL_SHIFT = 100.0f;
constexpr float HEADER_Y = -165.0f;
constexpr float INTERFACE_HEADER_Y = 25.0f;
constexpr float FIRST_ROW_Y = -120.0f;
constexpr float ROW_SPACING = 50.0f;
constexpr float UI_SCALE_ROW_Y = 70.0f;
constexpr float BUTTON_X = 140.0f;
constexpr float BUTTON_Y = 150.0f;
constexpr float VOLUME_MAX = 100.0f;
constexpr float VOLUME_STEP = 5.0f;

float rowAt(int index) noexcept
{
    return FIRST_ROW_Y + static_cast<float>(index) * ROW_SPACING;
}

std::string resolutionText(const events::Resolution &resolution)
{
    return std::to_string(resolution.width) + " x " +
           std::to_string(resolution.height);
}

std::vector<std::string> resolutionOptions()
{
    std::vector<std::string> options;

    options.reserve(settings::RESOLUTIONS.size());
    for (const events::Resolution &resolution : settings::RESOLUTIONS)
        options.push_back(resolutionText(resolution));
    return options;
}

std::vector<std::string> uiScaleOptions()
{
    std::vector<std::string> options;

    options.reserve(settings::UI_SCALE_PERCENTS.size());
    for (const int percent : settings::UI_SCALE_PERCENTS)
        options.push_back(std::to_string(percent) + " %");
    return options;
}

template <typename Container, typename Value>
std::size_t indexOf(const Container &container, const Value &value)
{
    const auto found = std::find(container.begin(), container.end(), value);

    return found == container.end() ? 0
                                    : static_cast<std::size_t>(std::distance(
                                          container.begin(), found));
}

std::string volumeText(const char *name, int value)
{
    return std::string(name) + " " + std::to_string(value) + " %";
}

} // namespace

SettingsScene::SettingsScene(engine::ecs::UISceneContext context,
                             settings::SettingsStore &store,
                             engine::ecs::UILayoutSystem &layout)
    : UIOverlay(std::move(context)),
      _store(store),
      _layout(layout)
{}

void SettingsScene::onExit()
{
    static_cast<void>(_store.save());
    UIOverlay::onExit();
}

Entity SettingsScene::control(SettingsControl control) const
{
    return _controls.at(static_cast<std::size_t>(control));
}

void SettingsScene::activate()
{
    _controls.clear();
    _volumeLabels.clear();
    addBackdrop(BACKDROP_COLOR);
    buildTitles();
    buildAudio();
    buildVideo();
    buildButtons();
    syncWidgets();
    widget<UISlider>(_controls.front()).isFocused = true;
    listenToWidgets();
}

void SettingsScene::buildTitles()
{
    addLabel("Settings", TITLE_OFFSET, TITLE_SIZE, TITLE_TEXT_SIZE);
    addLabel("Audio", {LEFT_COLUMN, HEADER_Y}, HEADER_SIZE, HEADER_TEXT_SIZE);
    addLabel("Video", {RIGHT_COLUMN, HEADER_Y}, HEADER_SIZE, HEADER_TEXT_SIZE);
    addLabel("Interface", {RIGHT_COLUMN, INTERFACE_HEADER_Y}, HEADER_SIZE,
             HEADER_TEXT_SIZE);
}

void SettingsScene::buildAudio()
{
    addVolumeRow(MASTER_ACTION, rowAt(0));
    addVolumeRow(MUSIC_ACTION, rowAt(1));
    addVolumeRow(SFX_ACTION, rowAt(2));
    addToggleRow("Mute", MUTE_ACTION, LEFT_COLUMN, rowAt(3));
}

void SettingsScene::buildVideo()
{
    addToggleRow("Fullscreen", FULLSCREEN_ACTION, RIGHT_COLUMN, rowAt(0));
    addDropdownRow("Resolution", RESOLUTION_ACTION, rowAt(1),
                   resolutionOptions());
    addToggleRow("VSync", VSYNC_ACTION, RIGHT_COLUMN, rowAt(2));
    addDropdownRow("UI scale", UI_SCALE_ACTION, UI_SCALE_ROW_Y,
                   uiScaleOptions());
}

void SettingsScene::buildButtons()
{
    _controls.push_back(addButton("Reset to defaults", RESET_ACTION,
                                  {-BUTTON_X, BUTTON_Y}, BUTTON_SIZE));
    _controls.push_back(
        addButton("Back", BACK_ACTION, {BUTTON_X, BUTTON_Y}, BUTTON_SIZE));
}

Entity SettingsScene::addRowLabel(const char *text, float column, float row)
{
    const Entity label =
        addLabel(text, {column - CELL_SHIFT, row}, LABEL_SIZE, ROW_TEXT_SIZE);

    widget<UIText>(label).align = UITextAlign::LEFT;
    return label;
}

void SettingsScene::addVolumeRow(const char *action, float row)
{
    const Entity entity =
        spawn(UIAnchor::CENTER, {LEFT_COLUMN + CELL_SHIFT, row}, CONTROL_SIZE);
    UISlider slider;

    slider.max = VOLUME_MAX;
    slider.step = VOLUME_STEP;
    slider.action = action;
    slider.style.fontId = fontId();
    registry().addComponent(entity, std::move(slider));
    _volumeLabels.push_back(addRowLabel("", LEFT_COLUMN, row));
    _controls.push_back(entity);
}

void SettingsScene::addToggleRow(const char *text, const char *action,
                                 float column, float row)
{
    const Entity entity =
        spawn(UIAnchor::CENTER, {column + CELL_SHIFT, row}, CONTROL_SIZE);
    UIToggle toggle;

    toggle.action = action;
    toggle.style.fontId = fontId();
    registry().addComponent(entity, std::move(toggle));
    addRowLabel(text, column, row);
    _controls.push_back(entity);
}

void SettingsScene::addDropdownRow(const char *text, const char *action,
                                   float row, std::vector<std::string> options)
{
    const Entity entity =
        spawn(UIAnchor::CENTER, {RIGHT_COLUMN + CELL_SHIFT, row}, CONTROL_SIZE);
    UIDropdown dropdown;

    dropdown.options = std::move(options);
    dropdown.action = action;
    dropdown.style.fontId = fontId();
    registry().addComponent(entity, std::move(dropdown));
    addRowLabel(text, RIGHT_COLUMN, row);
    _controls.push_back(entity);
}

void SettingsScene::listenToWidgets()
{
    listen<UIValueChangedEvent>(
        [this](const UIValueChangedEvent &event) { onValue(event); });
    listen<UISelectionChangedEvent>(
        [this](const UISelectionChangedEvent &event) { onSelection(event); });
    listen<UIButtonClickedEvent>(
        [this](const UIButtonClickedEvent &event) { onButton(event); });
    listen<UICancelEvent>([this](const UICancelEvent &) {
        bus().publish(engine::core::OverlayCloseRequested{});
    });
}

void SettingsScene::onValue(const UIValueChangedEvent &event)
{
    const int volume = static_cast<int>(std::lround(event.value));

    if (event.action == MASTER_ACTION)
        onVolume(events::VolumeBus::MASTER, volume);
    else if (event.action == MUSIC_ACTION)
        onVolume(events::VolumeBus::MUSIC, volume);
    else if (event.action == SFX_ACTION)
        onVolume(events::VolumeBus::SFX, volume);
    else
        onToggle(event.action, event.value > 0.5f);
}

void SettingsScene::onToggle(const std::string &action, bool checked)
{
    auto &settings = _store.settings();

    if (action == MUTE_ACTION) {
        settings.muted = checked;
        bus().publish(events::MuteChanged{checked});
    } else if (action == FULLSCREEN_ACTION) {
        settings.fullscreen = checked;
        publishVideo();
    } else if (action == VSYNC_ACTION) {
        settings.vsync = checked;
        publishVideo();
    }
}

void SettingsScene::onVolume(events::VolumeBus volumeBus, int value)
{
    auto &settings = _store.settings();

    if (volumeBus == events::VolumeBus::MASTER)
        settings.masterVolume = value;
    else if (volumeBus == events::VolumeBus::MUSIC)
        settings.musicVolume = value;
    else
        settings.sfxVolume = value;
    syncVolumeLabels();
    bus().publish(events::VolumeChanged{volumeBus, value});
}

void SettingsScene::onSelection(const UISelectionChangedEvent &event)
{
    auto &settings = _store.settings();

    if (event.action == RESOLUTION_ACTION) {
        settings.resolution = settings::RESOLUTIONS.at(event.index);
        publishVideo();
    } else if (event.action == UI_SCALE_ACTION) {
        settings.uiScalePercent = settings::UI_SCALE_PERCENTS.at(event.index);
        applyUIScale();
    }
}

void SettingsScene::onButton(const UIButtonClickedEvent &event)
{
    if (event.action == RESET_ACTION)
        reset();
    else if (event.action == BACK_ACTION)
        bus().publish(engine::core::OverlayCloseRequested{});
}

void SettingsScene::reset()
{
    const settings::KeyBindings bindings = _store.settings().bindings;

    _store.settings() = settings::ClientSettings{};
    _store.settings().bindings = bindings;
    syncWidgets();
    publishVolumes();
    bus().publish(events::MuteChanged{_store.settings().muted});
    publishVideo();
    applyUIScale();
}

void SettingsScene::syncWidgets()
{
    syncVolumes();
    syncVideo();
    syncVolumeLabels();
}

void SettingsScene::syncVolumes()
{
    const auto &settings = _store.settings();

    widget<UISlider>(control(SettingsControl::MASTER_VOLUME)).value =
        static_cast<float>(settings.masterVolume);
    widget<UISlider>(control(SettingsControl::MUSIC_VOLUME)).value =
        static_cast<float>(settings.musicVolume);
    widget<UISlider>(control(SettingsControl::SFX_VOLUME)).value =
        static_cast<float>(settings.sfxVolume);
    widget<UIToggle>(control(SettingsControl::MUTE)).checked = settings.muted;
}

void SettingsScene::syncVideo()
{
    const auto &settings = _store.settings();

    widget<UIToggle>(control(SettingsControl::FULLSCREEN)).checked =
        settings.fullscreen;
    widget<UIToggle>(control(SettingsControl::VSYNC)).checked = settings.vsync;
    widget<UIDropdown>(control(SettingsControl::RESOLUTION)).selected =
        indexOf(settings::RESOLUTIONS, settings.resolution);
    widget<UIDropdown>(control(SettingsControl::UI_SCALE)).selected =
        indexOf(settings::UI_SCALE_PERCENTS, settings.uiScalePercent);
}

void SettingsScene::syncVolumeLabels()
{
    const auto &settings = _store.settings();

    widget<UIText>(_volumeLabels.at(0)).text =
        volumeText("Master", settings.masterVolume);
    widget<UIText>(_volumeLabels.at(1)).text =
        volumeText("Music", settings.musicVolume);
    widget<UIText>(_volumeLabels.at(2)).text =
        volumeText("Effects", settings.sfxVolume);
}

void SettingsScene::publishVolumes()
{
    const auto &settings = _store.settings();

    bus().publish(events::VolumeChanged{events::VolumeBus::MASTER,
                                        settings.masterVolume});
    bus().publish(
        events::VolumeChanged{events::VolumeBus::MUSIC, settings.musicVolume});
    bus().publish(
        events::VolumeChanged{events::VolumeBus::SFX, settings.sfxVolume});
}

void SettingsScene::publishVideo()
{
    const auto &settings = _store.settings();

    bus().publish(events::VideoSettingsChanged{
        settings.fullscreen, settings.resolution, settings.vsync});
}

void SettingsScene::applyUIScale()
{
    _layout.setScale(static_cast<float>(_store.settings().uiScalePercent) /
                     100.0f);
}

void addSettingsRoute(SceneRouter &router,
                      const engine::ecs::UISceneContext &context,
                      settings::SettingsStore &store,
                      engine::ecs::UILayoutSystem &layout)
{
    router.setRoute(SceneRoute::SETTINGS, [context, &store, &layout] {
        return std::make_unique<SettingsScene>(context, store, layout);
    });
}

} // namespace client::scenes
