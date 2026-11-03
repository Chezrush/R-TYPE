/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** StatsOverlay.cpp
*/

#include "Client/Hud/StatsOverlay.hpp"

#include <utility>

#include "Client/Events/SettingsEvents.hpp"
#include "Engine/Ecs/Components/UIGraph.hpp"
#include "Engine/Ecs/Components/UIPanel.hpp"
#include "Engine/Ecs/Components/UIText.hpp"
#include "Engine/Ecs/Components/UITransform.hpp"

namespace client::hud {

namespace {

using engine::ecs::Entity;
using engine::ecs::UIColorRole;
using engine::ecs::components::UIAnchor;
using engine::ecs::components::UIColor;
using engine::ecs::components::UIGraph;
using engine::ecs::components::UIPanel;
using engine::ecs::components::UIText;
using engine::ecs::components::UITransform;

constexpr float REFRESH_SECONDS = 0.25f;
constexpr float RIGHT_GAP = 8.0f;
constexpr float TOP_GAP = 8.0f;
constexpr float PANEL_WIDTH = 300.0f;
constexpr float CONTENT_WIDTH = 284.0f;
constexpr float PADDING = 8.0f;
constexpr float LINE_HEIGHT = 20.0f;
constexpr float GRAPH_HEIGHT = 36.0f;
constexpr float GAP = 4.0f;
constexpr float EXTRA_LINE_HEIGHT = 18.0f;
constexpr unsigned int TEXT_SIZE = 14;
constexpr int Z_INDEX = 2000;
constexpr float FRAME_GRAPH_TOP_MS = 33.3f;
constexpr float RTT_GRAPH_TOP_MS = 0.0f;
constexpr float LOSS_GRAPH_TOP_PERCENT = 10.0f;
constexpr UIColor PANEL_COLOR{0, 0, 0, 170};
constexpr UIColor TEXT_COLOR{235, 235, 255, 255};

/** @brief y of each widget below the panel top, and its height. */
constexpr std::array<std::pair<float, float>, 7> ROWS{{
    {PADDING, LINE_HEIGHT},
    {PADDING + LINE_HEIGHT, GRAPH_HEIGHT},
    {PADDING + LINE_HEIGHT + GRAPH_HEIGHT + GAP, LINE_HEIGHT},
    {PADDING + 2 * LINE_HEIGHT + GRAPH_HEIGHT + GAP, GRAPH_HEIGHT},
    {PADDING + 2 * LINE_HEIGHT + 2 * GRAPH_HEIGHT + 2 * GAP, LINE_HEIGHT},
    {PADDING + 3 * LINE_HEIGHT + 2 * GRAPH_HEIGHT + 2 * GAP, GRAPH_HEIGHT},
    {PADDING + 3 * LINE_HEIGHT + 3 * GRAPH_HEIGHT + 3 * GAP, LINE_HEIGHT},
}};

constexpr float EXTRA_TOP =
    PADDING + 4 * LINE_HEIGHT + 3 * GRAPH_HEIGHT + 3 * GAP;

constexpr float panelHeight(std::size_t extraLines) noexcept
{
    return EXTRA_TOP + EXTRA_LINE_HEIGHT * static_cast<float>(extraLines) +
           PADDING;
}

} // namespace

StatsOverlay::StatsOverlay(engine::ecs::Registry &registry,
                           engine::core::EventBus &bus, std::string fontId)
    : _registry(registry),
      _bus(bus),
      _fontId(std::move(fontId))
{
    _listeners.push_back(_bus.subscribe<events::NetStatsUpdated>(
        [this](const events::NetStatsUpdated &stats) {
            _model.setNetStats(stats);
        }));
    _listeners.push_back(_bus.subscribe<events::StatsLine>(
        [this](const events::StatsLine &line) {
            _model.setLine(line.name, line.value);
        }));
    _listeners.push_back(_bus.subscribe<events::RawKeyPressed>(
        [this](const events::RawKeyPressed &event) {
            if (event.key == settings::KEY_F3)
                setVisible(!isVisible());
        }));
}

StatsOverlay::~StatsOverlay()
{
    for (const auto listener : _listeners)
        _bus.unsubscribe(listener);
    destroy();
}

void StatsOverlay::update(float dt)
{
    _model.addFrame(dt);
    if (!isVisible())
        return;
    refreshGraphs();
    _sinceRefresh += dt;
    if (_sinceRefresh >= REFRESH_SECONDS) {
        _sinceRefresh = 0.0f;
        refreshTexts();
    }
}

void StatsOverlay::setVisible(bool visible)
{
    if (visible == isVisible())
        return;
    if (!visible) {
        destroy();
        return;
    }
    build();
    refreshGraphs();
    refreshTexts();
}

bool StatsOverlay::isVisible() const noexcept
{
    return !_widgets.empty();
}

const stats::StatsModel &StatsOverlay::model() const noexcept
{
    return _model;
}

Entity StatsOverlay::widget(StatsWidget which) const
{
    return _widgets.at(static_cast<std::size_t>(which));
}

Entity StatsOverlay::extraLine(std::size_t index) const
{
    return _extraLines.at(index);
}

void StatsOverlay::build()
{
    const Entity panel = spawn(0.0f, panelHeight(0));
    UIPanel background;

    background.color = PANEL_COLOR;
    _registry.getComponents<UITransform>().get(panel)->size.x = PANEL_WIDTH;
    _registry.addComponent(panel, background);
    _widgets.push_back(panel);
    for (const auto &[y, height] : ROWS)
        _widgets.push_back(spawn(y, height));
    buildContents();
}

void StatsOverlay::buildContents()
{
    for (const StatsWidget text :
         {StatsWidget::FRAME_TEXT, StatsWidget::RTT_TEXT,
          StatsWidget::LOSS_TEXT, StatsWidget::TRAFFIC_TEXT})
        addText(widget(text));
    addGraph(widget(StatsWidget::FRAME_GRAPH), FRAME_GRAPH_TOP_MS);
    addGraph(widget(StatsWidget::RTT_GRAPH), RTT_GRAPH_TOP_MS);
    addGraph(widget(StatsWidget::LOSS_GRAPH), LOSS_GRAPH_TOP_PERCENT);
    for (std::size_t i = 0; i < stats::StatsModel::MAX_EXTRA_LINES; ++i) {
        _extraLines.push_back(
            spawn(EXTRA_TOP + EXTRA_LINE_HEIGHT * static_cast<float>(i),
                  EXTRA_LINE_HEIGHT));
        addText(_extraLines.back());
    }
}

void StatsOverlay::destroy()
{
    for (const auto *group : {&_widgets, &_extraLines}) {
        for (const Entity entity : *group)
            _registry.killEntity(entity);
    }
    _widgets.clear();
    _extraLines.clear();
}

Entity StatsOverlay::spawn(float y, float height)
{
    const Entity entity = _registry.spawnEntity();
    UITransform transform;
    const bool panel = _widgets.empty() && y == 0.0f;

    transform.anchor = UIAnchor::TOP_RIGHT;
    transform.offset = {panel ? -RIGHT_GAP : -RIGHT_GAP - PADDING, TOP_GAP + y};
    transform.size = {CONTENT_WIDTH, height};
    transform.zIndex = panel ? Z_INDEX : Z_INDEX + 1;
    _registry.addComponent(entity, transform);
    return entity;
}

void StatsOverlay::addText(Entity entity)
{
    UIText text;

    text.fontId = _fontId;
    text.characterSize = TEXT_SIZE;
    text.color = TEXT_COLOR;
    text.colorRole = UIColorRole::TEXT;
    _registry.addComponent(entity, std::move(text));
}

void StatsOverlay::addGraph(Entity entity, float maxValue)
{
    UIGraph graph;

    graph.capacity = stats::StatsModel::SAMPLES;
    graph.maxValue = maxValue;
    graph.barRole = UIColorRole::GAUGE;
    _registry.addComponent(entity, std::move(graph));
}

void StatsOverlay::refreshTexts()
{
    const std::vector<std::string> extras = _model.extraLines();

    setText(widget(StatsWidget::FRAME_TEXT), _model.frameText());
    setText(widget(StatsWidget::RTT_TEXT), _model.rttText());
    setText(widget(StatsWidget::LOSS_TEXT), _model.lossText());
    setText(widget(StatsWidget::TRAFFIC_TEXT), _model.trafficText());
    for (std::size_t i = 0; i < _extraLines.size(); ++i)
        setText(_extraLines.at(i), i < extras.size() ? extras.at(i) : "");
    _registry.getComponents<UITransform>()
        .get(widget(StatsWidget::PANEL))
        ->size.y = panelHeight(extras.size());
}

void StatsOverlay::refreshGraphs()
{
    setSamples(widget(StatsWidget::FRAME_GRAPH), _model.frameTimes());
    setSamples(widget(StatsWidget::RTT_GRAPH), _model.rtts());
    setSamples(widget(StatsWidget::LOSS_GRAPH), _model.losses());
}

void StatsOverlay::setText(Entity entity, const std::string &text)
{
    _registry.getComponents<UIText>().get(entity)->text = text;
}

void StatsOverlay::setSamples(Entity entity, const stats::RollingSeries &series)
{
    _registry.getComponents<UIGraph>().get(entity)->samples = series.values();
}

} // namespace client::hud
