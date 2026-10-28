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
#include "Engine/Core/Vec2.hpp"
#include "Engine/Ecs/Components/UIColor.hpp"
#include "Engine/Ecs/Components/UIPanel.hpp"
#include "Engine/Ecs/Components/UIProgressBar.hpp"
#include "Engine/Ecs/Components/UIText.hpp"
#include "Engine/Ecs/Components/UITransform.hpp"
#include "Engine/Ecs/Entity.hpp"
#include "Engine/Ecs/Registry.hpp"

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
 * tag that follows its ship. The widgets are entities of the game registry
 * (the UI components must be registered, see registerUIComponents()); the
 * game's UILayoutSystem resolves them and engine::render::UIRenderSystem
 * draws them in screen space, so the HUD stays pinned while the world
 * scrolls. The entities are killed when the view is destroyed, so the
 * registry must outlive it. This class needs no SFML.
 */
class HudView {
public:
    explicit HudView(engine::ecs::Registry &registry, std::string fontId,
                     std::string shipTextureId = "sprites/r-typesheet42.gif",
                     float width = 1280.0f);
    ~HudView();

    HudView(const HudView &) = delete;
    HudView &operator=(const HudView &) = delete;
    HudView(HudView &&) = delete;
    HudView &operator=(HudView &&) = delete;

    /** @brief Stretches the bottom bar to a new window width. */
    void resize(float width);

    /** @brief Copies the state into the widgets. */
    void sync(const HudState &state);

    /**
     * @brief Places a player tag above its ship, or hides it.
     * @param shipTop Screen position of the middle of the ship's top edge.
     */
    void setMarker(std::size_t player,
                   std::optional<engine::core::Vec2> shipTop);

    [[nodiscard]] engine::ecs::Entity widget(HudWidget widget) const;
    [[nodiscard]] engine::ecs::Entity tag(std::size_t player) const;
    [[nodiscard]] engine::ecs::Entity tagOutline(std::size_t player) const;
    [[nodiscard]] engine::ecs::Entity marker(std::size_t player) const;
    [[nodiscard]] engine::ecs::Entity markerOutline(std::size_t player) const;

    /** @brief Arcade score: 7 digits with leading zeros, capped. */
    [[nodiscard]] static std::string formatScore(std::uint32_t score);

    /** @brief "P1" to "P4". */
    [[nodiscard]] static std::string playerTag(std::size_t player);

    /** @brief Color of a player, matching its ship in the sprite sheet. */
    [[nodiscard]] static engine::ecs::components::UIColor
    playerColor(std::size_t player);

private:
    template <typename Component>
    Component &component(engine::ecs::Entity entity);

    engine::ecs::Entity spawn(engine::ecs::components::UIAnchor anchor,
                              engine::core::Vec2 offset,
                              engine::core::Vec2 size, int zIndex);
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

    engine::ecs::Registry &_registry;
    std::string _fontId;
    std::string _shipTextureId;
    std::vector<engine::ecs::Entity> _widgets;
    std::vector<engine::ecs::Entity> _tags;
    std::vector<engine::ecs::Entity> _tagOutlines;
    std::vector<engine::ecs::Entity> _markers;
    std::vector<engine::ecs::Entity> _markerOutlines;
    std::array<std::optional<engine::core::Vec2>, MAX_PLAYERS>
        _markerPositions{};
};

} // namespace client::hud

#endif /* !HUDVIEW_HPP_ */
