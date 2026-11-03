/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** StatsOverlay.hpp
*/

#ifndef STATSOVERLAY_HPP_
#define STATSOVERLAY_HPP_

#include <optional>
#include <string>
#include <vector>

#include "Client/Stats/StatsModel.hpp"
#include "Engine/Core/EventBus.hpp"
#include "Engine/Ecs/Entity.hpp"
#include "Engine/Ecs/Registry.hpp"

namespace client::hud {

/** @brief The widgets of the stats overlay, top to bottom. */
enum class StatsWidget : std::uint8_t {
    PANEL,
    FRAME_TEXT,
    FRAME_GRAPH,
    RTT_TEXT,
    RTT_GRAPH,
    LOSS_TEXT,
    LOSS_GRAPH,
    TRAFFIC_TEXT,
    COUNT
};

/**
 * @brief Performance and network stats, pinned to the top right.
 *
 * Not a scene: it never takes the input, so the game goes on. F3
 * (RawKeyPressed with KEY_F3) shows or hides it. update() gets the real frame
 * time from the render loop, also while hidden, so the graphs are full when
 * shown. The texts refresh 4 times per second, the graphs every frame. The
 * network values come from NetStatsUpdated, extra lines from StatsLine. The
 * registry and the bus must outlive this object.
 */
class StatsOverlay {
public:
    StatsOverlay(engine::ecs::Registry &registry, engine::core::EventBus &bus,
                 std::string fontId);
    ~StatsOverlay();

    StatsOverlay(const StatsOverlay &) = delete;
    StatsOverlay &operator=(const StatsOverlay &) = delete;
    StatsOverlay(StatsOverlay &&) = delete;
    StatsOverlay &operator=(StatsOverlay &&) = delete;

    /** @brief Records a frame and refreshes the widgets if shown. */
    void update(float dt);

    void setVisible(bool visible);
    [[nodiscard]] bool isVisible() const noexcept;

    [[nodiscard]] const stats::StatsModel &model() const noexcept;
    [[nodiscard]] engine::ecs::Entity widget(StatsWidget which) const;
    [[nodiscard]] engine::ecs::Entity extraLine(std::size_t index) const;

private:
    void build();
    void buildContents();
    void destroy();
    engine::ecs::Entity spawn(float y, float height);
    void addText(engine::ecs::Entity entity);
    void addGraph(engine::ecs::Entity entity, float maxValue);
    void refreshTexts();
    void refreshGraphs();
    void setText(engine::ecs::Entity entity, const std::string &text);
    void setSamples(engine::ecs::Entity entity,
                    const stats::RollingSeries &series);

    engine::ecs::Registry &_registry;
    engine::core::EventBus &_bus;
    std::string _fontId;
    std::vector<engine::core::EventBus::ListenerId> _listeners;
    stats::StatsModel _model;
    std::vector<engine::ecs::Entity> _widgets;
    std::vector<engine::ecs::Entity> _extraLines;
    float _sinceRefresh = 0.0f;
};

} // namespace client::hud

#endif /* !STATSOVERLAY_HPP_ */
