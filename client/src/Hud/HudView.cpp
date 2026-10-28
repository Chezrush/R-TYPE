/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** HudView.cpp
*/

#include "Client/Hud/HudView.hpp"

#include <algorithm>
#include <utility>

namespace client::hud {

namespace {

using engine::core::Vec2;
using engine::ecs::Entity;
using engine::ecs::components::UIAnchor;
using engine::ecs::components::UIColor;
using engine::ecs::components::UIPanel;
using engine::ecs::components::UIProgressBar;
using engine::ecs::components::UIText;
using engine::ecs::components::UITextAlign;
using engine::ecs::components::UITransform;

struct WidgetLayout {
    UIAnchor anchor;
    Vec2 offset;
    Vec2 size;
    int zIndex;
};

constexpr float BAR_HEIGHT = 48.0f;
constexpr unsigned int BAR_TEXT_SIZE = 20;
constexpr unsigned int MARKER_TEXT_SIZE = 14;
constexpr std::uint32_t MAX_SCORE = 9'999'999;
constexpr std::size_t SCORE_DIGITS = 7;
constexpr float SHIP_FRAME_WIDTH = 33.0f;
constexpr float SHIP_FRAME_HEIGHT = 17.0f;
constexpr float SHIP_FRAME_COLUMN = 2.0f;
constexpr Vec2 TAG_SIZE{38.0f, 28.0f};
constexpr float TAG_SPACING = 44.0f;
constexpr Vec2 MARKER_SIZE{34.0f, 20.0f};
constexpr float MARKER_GAP = 6.0f;
constexpr float OUTLINE = 2.0f;

constexpr UIColor BAR_COLOR{0, 0, 0, 255};
constexpr UIColor EDGE_COLOR{40, 70, 200, 255};
constexpr UIColor TEXT_COLOR{235, 235, 255, 255};
constexpr UIColor SCORE_COLOR{120, 180, 255, 255};
constexpr UIColor BEAM_FRAME_COLOR{70, 110, 255, 255};
constexpr UIColor BEAM_FILL_COLOR{110, 190, 255, 255};
constexpr UIColor BEAM_FULL_COLOR{255, 255, 255, 255};
constexpr UIColor OUTLINE_COLOR{255, 255, 255, 255};
constexpr UIColor HIDDEN{0, 0, 0, 0};
constexpr UIColor OFFLINE_TAG_COLOR{45, 45, 55, 255};
constexpr UIColor OFFLINE_TEXT_COLOR{110, 110, 120, 255};

constexpr std::array<UIColor, MAX_PLAYERS> PLAYER_COLORS{{
    {0, 190, 230, 255},
    {210, 70, 210, 255},
    {40, 200, 70, 255},
    {230, 70, 60, 255},
}};

constexpr float centeredInBar(float height) noexcept
{
    return -(BAR_HEIGHT - height) / 2.0f;
}

constexpr std::array<WidgetLayout, static_cast<std::size_t>(HudWidget::COUNT)>
    LAYOUT{{
        {UIAnchor::BOTTOM_LEFT, {0.0f, 0.0f}, {1280.0f, BAR_HEIGHT}, 0},
        {UIAnchor::BOTTOM_LEFT, {0.0f, -BAR_HEIGHT}, {1280.0f, OUTLINE}, 1},
        {UIAnchor::BOTTOM_LEFT,
         {200.0f, centeredInBar(24.0f)},
         {46.0f, 24.0f},
         1},
        {UIAnchor::BOTTOM_LEFT,
         {252.0f, centeredInBar(28.0f)},
         {40.0f, 28.0f},
         1},
        {UIAnchor::BOTTOM_LEFT,
         {300.0f, centeredInBar(28.0f)},
         {120.0f, 28.0f},
         1},
        {UIAnchor::BOTTOM_CENTER,
         {-170.0f, centeredInBar(28.0f)},
         {80.0f, 28.0f},
         1},
        {UIAnchor::BOTTOM_CENTER,
         {30.0f, centeredInBar(20.0f)},
         {304.0f, 20.0f},
         1},
        {UIAnchor::BOTTOM_CENTER,
         {30.0f, centeredInBar(16.0f)},
         {300.0f, 16.0f},
         2},
        {UIAnchor::BOTTOM_RIGHT,
         {-16.0f, centeredInBar(28.0f)},
         {200.0f, 28.0f},
         1},
    }};

UIPanel colorPanel(UIColor color)
{
    UIPanel panel;

    panel.color = color;
    return panel;
}

} // namespace

template <typename Component>
Component &HudView::component(Entity entity)
{
    return *_registry.getComponents<Component>().get(entity);
}

HudView::HudView(engine::ecs::Registry &registry, std::string fontId,
                 std::string shipTextureId, float width)
    : _registry(registry),
      _fontId(std::move(fontId)),
      _shipTextureId(std::move(shipTextureId))
{
    buildBar();
    buildPlayerTags();
    buildMarkers();
    sync(HudState{});
    resize(width);
}

HudView::~HudView()
{
    for (const auto *group :
         {&_widgets, &_tags, &_tagOutlines, &_markers, &_markerOutlines}) {
        for (const Entity entity : *group)
            _registry.killEntity(entity);
    }
}

void HudView::resize(float width)
{
    for (const HudWidget bar : {HudWidget::BAR, HudWidget::BAR_EDGE})
        component<UITransform>(widget(bar)).size.x = width;
}

void HudView::sync(const HudState &state)
{
    syncTags(state);
    syncCounters(state);
    syncMarkers(state);
}

void HudView::setMarker(std::size_t player, std::optional<Vec2> shipTop)
{
    if (player >= MAX_PLAYERS)
        return;
    _markerPositions.at(player) = shipTop;
    if (!shipTop.has_value())
        return;
    const Vec2 offset{shipTop->x - MARKER_SIZE.x / 2.0f,
                      shipTop->y - MARKER_SIZE.y - MARKER_GAP};
    component<UITransform>(_markers.at(player)).offset = offset;
    component<UITransform>(_markerOutlines.at(player)).offset = {
        offset.x - OUTLINE, offset.y - OUTLINE};
}

Entity HudView::widget(HudWidget widget) const
{
    return _widgets.at(static_cast<std::size_t>(widget));
}

Entity HudView::tag(std::size_t player) const
{
    return _tags.at(player);
}

Entity HudView::tagOutline(std::size_t player) const
{
    return _tagOutlines.at(player);
}

Entity HudView::marker(std::size_t player) const
{
    return _markers.at(player);
}

Entity HudView::markerOutline(std::size_t player) const
{
    return _markerOutlines.at(player);
}

std::string HudView::formatScore(std::uint32_t score)
{
    const std::string digits = std::to_string(std::min(score, MAX_SCORE));

    return std::string(SCORE_DIGITS - digits.size(), '0') + digits;
}

std::string HudView::playerTag(std::size_t player)
{
    return "P" + std::to_string(player + 1);
}

UIColor HudView::playerColor(std::size_t player)
{
    return PLAYER_COLORS.at(player % MAX_PLAYERS);
}

Entity HudView::spawn(UIAnchor anchor, Vec2 offset, Vec2 size, int zIndex)
{
    const Entity entity = _registry.spawnEntity();
    UITransform transform;

    transform.anchor = anchor;
    transform.offset = offset;
    transform.size = size;
    transform.zIndex = zIndex;
    _registry.addComponent(entity, transform);
    return entity;
}

UIText HudView::label(std::string text, UITextAlign align,
                      unsigned int characterSize) const
{
    return UIText{std::move(text), _fontId, characterSize, TEXT_COLOR, align};
}

void HudView::buildBar()
{
    for (const WidgetLayout &layout : LAYOUT)
        _widgets.push_back(
            spawn(layout.anchor, layout.offset, layout.size, layout.zIndex));
    _registry.addComponent(widget(HudWidget::BAR), colorPanel(BAR_COLOR));
    _registry.addComponent(widget(HudWidget::BAR_EDGE), colorPanel(EDGE_COLOR));
    _registry.addComponent(widget(HudWidget::BEAM_FRAME),
                           colorPanel(BEAM_FRAME_COLOR));
    UIPanel icon = colorPanel(OUTLINE_COLOR);
    icon.textureId = _shipTextureId;
    _registry.addComponent(widget(HudWidget::LIVES_ICON), icon);
    buildBarTexts();
}

void HudView::buildBarTexts()
{
    _registry.addComponent(widget(HudWidget::LIVES),
                           label("", UITextAlign::LEFT, BAR_TEXT_SIZE));
    _registry.addComponent(widget(HudWidget::STAGE),
                           label("", UITextAlign::LEFT, BAR_TEXT_SIZE));
    _registry.addComponent(widget(HudWidget::BEAM_LABEL),
                           label("BEAM", UITextAlign::RIGHT, BAR_TEXT_SIZE));
    _registry.addComponent(
        widget(HudWidget::BEAM_GAUGE),
        UIProgressBar{0.0f, 1.0f, 0.0f, {}, BAR_COLOR, BEAM_FILL_COLOR});
    _registry.addComponent(widget(HudWidget::SCORE),
                           label("", UITextAlign::RIGHT, BAR_TEXT_SIZE));
}

void HudView::buildPlayerTags()
{
    for (std::size_t i = 0; i < MAX_PLAYERS; ++i) {
        const float x = 16.0f + TAG_SPACING * static_cast<float>(i);
        const float y = centeredInBar(TAG_SIZE.y);
        _tagOutlines.push_back(
            spawn(UIAnchor::BOTTOM_LEFT, {x - OUTLINE, y + OUTLINE},
                  {TAG_SIZE.x + 2 * OUTLINE, TAG_SIZE.y + 2 * OUTLINE}, 2));
        _tags.push_back(spawn(UIAnchor::BOTTOM_LEFT, {x, y}, TAG_SIZE, 3));
        _registry.addComponent(_tagOutlines.back(), colorPanel(HIDDEN));
        _registry.addComponent(_tags.back(), colorPanel(OFFLINE_TAG_COLOR));
        _registry.addComponent(
            _tags.back(),
            label(playerTag(i), UITextAlign::CENTER, BAR_TEXT_SIZE));
    }
}

void HudView::buildMarkers()
{
    const Vec2 outlineSize{MARKER_SIZE.x + 2 * OUTLINE,
                           MARKER_SIZE.y + 2 * OUTLINE};

    for (std::size_t i = 0; i < MAX_PLAYERS; ++i) {
        _markerOutlines.push_back(
            spawn(UIAnchor::TOP_LEFT, {}, outlineSize, 10));
        _markers.push_back(spawn(UIAnchor::TOP_LEFT, {}, MARKER_SIZE, 11));
        _registry.addComponent(_markerOutlines.back(), colorPanel(HIDDEN));
        _registry.addComponent(_markers.back(), colorPanel(HIDDEN));
        _registry.addComponent(
            _markers.back(), label("", UITextAlign::CENTER, MARKER_TEXT_SIZE));
    }
}

void HudView::syncTags(const HudState &state)
{
    for (std::size_t i = 0; i < MAX_PLAYERS; ++i) {
        const bool online = state.connected.at(i);
        const bool local = state.localPlayer == i;
        setPanelColor(_tags.at(i), online ? playerColor(i) : OFFLINE_TAG_COLOR);
        setText(_tags.at(i), playerTag(i),
                online ? TEXT_COLOR : OFFLINE_TEXT_COLOR);
        setPanelColor(_tagOutlines.at(i), local ? OUTLINE_COLOR : HIDDEN);
    }
}

void HudView::syncCounters(const HudState &state)
{
    const float row = static_cast<float>(state.localPlayer.value_or(0));
    auto &icon = component<UIPanel>(widget(HudWidget::LIVES_ICON));
    auto &beam = component<UIProgressBar>(widget(HudWidget::BEAM_GAUGE));

    icon.textureRect = {SHIP_FRAME_COLUMN * SHIP_FRAME_WIDTH,
                        row * SHIP_FRAME_HEIGHT, SHIP_FRAME_WIDTH,
                        SHIP_FRAME_HEIGHT};
    setText(widget(HudWidget::LIVES), std::to_string(state.lives), TEXT_COLOR);
    setText(widget(HudWidget::STAGE), "STAGE " + std::to_string(state.level),
            TEXT_COLOR);
    setText(widget(HudWidget::SCORE), formatScore(state.score), SCORE_COLOR);
    beam.value = state.beamCharge;
    beam.fillColor =
        state.beamCharge >= 1.0f ? BEAM_FULL_COLOR : BEAM_FILL_COLOR;
}

void HudView::syncMarkers(const HudState &state)
{
    for (std::size_t i = 0; i < MAX_PLAYERS; ++i) {
        const bool shown =
            state.connected.at(i) && _markerPositions.at(i).has_value();
        const bool local = shown && state.localPlayer == i;
        setPanelColor(_markers.at(i), shown ? playerColor(i) : HIDDEN);
        setText(_markers.at(i), shown ? playerTag(i) : "", TEXT_COLOR);
        setPanelColor(_markerOutlines.at(i), local ? OUTLINE_COLOR : HIDDEN);
    }
}

void HudView::setPanelColor(Entity entity, UIColor color)
{
    component<UIPanel>(entity).color = color;
}

void HudView::setText(Entity entity, std::string text, UIColor color)
{
    auto &widgetText = component<UIText>(entity);

    widgetText.text = std::move(text);
    widgetText.color = color;
}

} // namespace client::hud
