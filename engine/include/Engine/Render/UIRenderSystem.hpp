/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIRenderSystem.hpp
*/

#ifndef UIRENDERSYSTEM_HPP_
#define UIRENDERSYSTEM_HPP_

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstddef>
#include <vector>

#include "Engine/Ecs/Components/UIPanel.hpp"
#include "Engine/Ecs/Components/UIProgressBar.hpp"
#include "Engine/Ecs/Components/UIText.hpp"
#include "Engine/Ecs/Components/UITransform.hpp"
#include "Engine/Ecs/SparseArray.hpp"
#include "Engine/Resources/ResourceManager.hpp"

namespace engine::render {

/**
 * @brief Draws the UI entities in screen space, sorted by zIndex.
 *
 * An entity is drawn when it has a UITransform and at least one of UIPanel,
 * UIProgressBar or UIText, in that order, inside its actualBounds. The UI
 * uses its own view, one unit per pixel of the target, so it does not move
 * with the world camera. The target view is restored afterwards.
 */
class UIRenderSystem {
public:
    explicit UIRenderSystem(resources::ResourceManager &resources) noexcept;

    /** @brief Draws every UI entity on the target. */
    void
    render(sf::RenderTarget &target,
           const ecs::SparseArray<ecs::components::UITransform> &transforms,
           const ecs::SparseArray<ecs::components::UIPanel> &panels,
           const ecs::SparseArray<ecs::components::UIProgressBar> &bars,
           const ecs::SparseArray<ecs::components::UIText> &texts);

    /** @brief Fixed view mapping one unit to one pixel of the target. */
    [[nodiscard]] static sf::View screenView(sf::Vector2u targetSize);

    /** @brief Indices of the drawable entities, by zIndex then index. */
    [[nodiscard]] static std::vector<std::size_t>
    drawOrder(const ecs::SparseArray<ecs::components::UITransform> &transforms,
              const ecs::SparseArray<ecs::components::UIPanel> &panels,
              const ecs::SparseArray<ecs::components::UIProgressBar> &bars,
              const ecs::SparseArray<ecs::components::UIText> &texts);

    /** @brief Filled part of the bar, in [0, 1]. */
    [[nodiscard]] static float
    fillRatio(const ecs::components::UIProgressBar &bar) noexcept;

    /** @brief Rectangle covered by the filled part of the bar. */
    [[nodiscard]] static ecs::components::Rect
    fillBounds(const ecs::components::UIProgressBar &bar,
               const ecs::components::Rect &bounds) noexcept;

    /** @brief Textured triangles of a panel, 6 (STRETCH) or 54 (9-slice). */
    [[nodiscard]] static sf::VertexArray
    panelVertices(const ecs::components::UIPanel &panel,
                  const ecs::components::Rect &bounds,
                  sf::Vector2u textureSize);

    /** @brief Point of the bounds the text is aligned on. */
    [[nodiscard]] static sf::Vector2f
    textAnchor(ecs::components::UITextAlign align,
               const ecs::components::Rect &bounds) noexcept;

private:
    void drawPanel(sf::RenderTarget &target,
                   const ecs::components::UIPanel &panel,
                   const ecs::components::Rect &bounds);

    void drawText(sf::RenderTarget &target, const ecs::components::UIText &text,
                  const ecs::components::Rect &bounds);

    resources::ResourceManager &_resources;
};

} // namespace engine::render

#endif /* !UIRENDERSYSTEM_HPP_ */
