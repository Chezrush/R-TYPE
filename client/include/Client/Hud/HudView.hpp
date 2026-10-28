/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** HudView.hpp
*/

#ifndef HUDVIEW_HPP_
#define HUDVIEW_HPP_

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "Client/Hud/HudState.hpp"
#include "Engine/Ecs/Components/UIColor.hpp"
#include "Engine/Ecs/Components/UIPanel.hpp"
#include "Engine/Ecs/Components/UIProgressBar.hpp"
#include "Engine/Ecs/Components/UIText.hpp"
#include "Engine/Ecs/Components/UITransform.hpp"
#include "Engine/Ecs/Entity.hpp"
#include "Engine/Ecs/EntityAllocator.hpp"
#include "Engine/Ecs/SparseArray.hpp"
#include "Engine/Ecs/Systems/UILayoutSystem.hpp"

namespace client::hud {

/**
 * @brief Fixed widgets of the bottom bar.
 */
enum class HudWidget : std::uint8_t {
    BAR,
    BAR_EDGE,
    LIVES_ICON,
    LIVES,
    STAGE,
    BEAM_LABEL,
    BEAM_FRAME,
    BEAM_GAUGE,
    SCORE,
    COUNT
};

/**
 * @brief Arcade R-Type HUD, built from the engine UI components.
 *
 * Bottom bar: player tags P1-P4 (the local one outlined in white), ship icon
 * and lives, stage (level), BEAM gauge, 7 digit score. Each player also gets a
 * tag that follows its ship. Everything is in screen space: draw the arrays
 * with engine::render::UIRenderSystem and the HUD stays pinned while the world
 * scrolls. This class needs no SFML.
 */
class HudView {
public:
    explicit HudView(std::string fontId,
                     std::string shipTextureId = "sprites/r-typesheet42.gif",
                     float width = 1280.0f, float height = 720.0f);

    /** @brief Lays the HUD out again for a new window size. */
    void resize(float width, float height);

    /** @brief Copies the state into the widgets. */
    void sync(const HudState &state);

    /**
     * @brief Places a player tag above its ship, or hides it.
     * @param shipTop Screen position of the middle of the ship's top edge.
     */
    void setMarker(std::size_t player,
                   std::optional<engine::ecs::components::Vec2> shipTop);

    [[nodiscard]] engine::ecs::Entity widget(HudWidget widget) const;
    [[nodiscard]] engine::ecs::Entity tag(std::size_t player) const;
    [[nodiscard]] engine::ecs::Entity tagOutline(std::size_t player) const;
    [[nodiscard]] engine::ecs::Entity marker(std::size_t player) const;
    [[nodiscard]] engine::ecs::Entity markerOutline(std::size_t player) const;

    [[nodiscard]] const engine::ecs::SparseArray<
        engine::ecs::components::UITransform> &
    transforms() const noexcept;
    [[nodiscard]] const engine::ecs::SparseArray<
        engine::ecs::components::UIPanel> &
    panels() const noexcept;
    [[nodiscard]] const engine::ecs::SparseArray<
        engine::ecs::components::UIProgressBar> &
    bars() const noexcept;
    [[nodiscard]] const engine::ecs::SparseArray<
        engine::ecs::components::UIText> &
    texts() const noexcept;

    /** @brief Arcade score: 7 digits with leading zeros, capped. */
    [[nodiscard]] static std::string formatScore(std::uint32_t score);

    /** @brief "P1" to "P4". */
    [[nodiscard]] static std::string playerTag(std::size_t player);

    /** @brief Color of a player, matching its ship in the sprite sheet. */
    [[nodiscard]] static engine::ecs::components::UIColor
    playerColor(std::size_t player);

private:
    engine::ecs::Entity spawn(engine::ecs::components::UIAnchor anchor,
                              engine::ecs::components::Vec2 offset,
                              engine::ecs::components::Vec2 size, int zIndex);
    [[nodiscard]] engine::ecs::components::UIText
    label(std::string text, engine::ecs::components::UITextAlign align,
          unsigned int characterSize) const;
    void buildBar();
    void buildBarTexts();
    void buildPlayerTags();
    void buildMarkers();
    void syncTags(const HudState &state);
    void syncCounters(const HudState &state);
    void syncMarkers(const HudState &state);
    void setPanelColor(engine::ecs::Entity entity,
                       engine::ecs::components::UIColor color);
    void setText(engine::ecs::Entity entity, std::string text,
                 engine::ecs::components::UIColor color);

    std::string _fontId;
    std::string _shipTextureId;
    engine::ecs::EntityAllocator _allocator;
    engine::ecs::SparseArray<engine::ecs::components::UITransform> _transforms;
    engine::ecs::SparseArray<engine::ecs::components::UIPanel> _panels;
    engine::ecs::SparseArray<engine::ecs::components::UIProgressBar> _bars;
    engine::ecs::SparseArray<engine::ecs::components::UIText> _texts;
    engine::ecs::UILayoutSystem _layout;
    std::vector<engine::ecs::Entity> _widgets;
    std::vector<engine::ecs::Entity> _tags;
    std::vector<engine::ecs::Entity> _tagOutlines;
    std::vector<engine::ecs::Entity> _markers;
    std::vector<engine::ecs::Entity> _markerOutlines;
    std::array<std::optional<engine::ecs::components::Vec2>, MAX_PLAYERS>
        _markerPositions{};
};

} // namespace client::hud

#endif /* !HUDVIEW_HPP_ */
