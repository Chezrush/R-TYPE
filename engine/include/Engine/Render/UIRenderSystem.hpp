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
#include <string>
#include <vector>

#include "Engine/Ecs/Components/UIDropdown.hpp"
#include "Engine/Ecs/Components/UIPanel.hpp"
#include "Engine/Ecs/Components/UIProgressBar.hpp"
#include "Engine/Ecs/Components/UIScrollList.hpp"
#include "Engine/Ecs/Components/UIText.hpp"
#include "Engine/Ecs/Components/UITextInput.hpp"
#include "Engine/Ecs/Components/UITransform.hpp"
#include "Engine/Ecs/Components/UIWidgetStyle.hpp"
#include "Engine/Ecs/Entity.hpp"
#include "Engine/Ecs/Registry.hpp"
#include "Engine/Resources/ResourceManager.hpp"

namespace engine::render {

/**
 * @brief Draws the UI entities in screen space, sorted by zIndex.
 *
 * An entity is drawn when it has a UITransform and at least one of UIPanel,
 * UIProgressBar, UIText or an interactive widget, in that order, inside its
 * actualBounds. A focused widget gets an outline; an open dropdown list is
 * drawn last, on top of everything. The UI
 * uses its own view, one unit per pixel of the target, so it does not move
 * with the world camera. The target view is restored afterwards.
 */
class UIRenderSystem {
public:
    explicit UIRenderSystem(resources::ResourceManager &resources) noexcept;

    /**
     * @brief Draws every UI entity of the registry on the target.
     *
     * The UI components must be registered (see registerUIComponents()).
     */
    void render(sf::RenderTarget &target, const ecs::Registry &registry);

    /** @brief Fixed view mapping one unit to one pixel of the target. */
    [[nodiscard]] static sf::View screenView(sf::Vector2u targetSize);

    /** @brief Drawable entities, by zIndex then entity order. */
    [[nodiscard]] static std::vector<ecs::Entity>
    drawOrder(const ecs::Registry &registry);

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
    void drawEntity(sf::RenderTarget &target, const ecs::Registry &registry,
                    ecs::Entity entity);

    void drawPanel(sf::RenderTarget &target,
                   const ecs::components::UIPanel &panel,
                   const ecs::components::Rect &bounds);

    [[nodiscard]] unsigned int scaledTextSize(unsigned int size) const noexcept;
    void drawText(sf::RenderTarget &target, const ecs::components::UIText &text,
                  const ecs::components::Rect &bounds);

    void drawWidgets(sf::RenderTarget &target, const ecs::Registry &registry,
                     ecs::Entity entity, const ecs::components::Rect &bounds);
    void drawOpenDropdowns(sf::RenderTarget &target,
                           const ecs::Registry &registry);
    void drawTextInput(sf::RenderTarget &target,
                       const ecs::components::UITextInput &input,
                       const ecs::components::Rect &bounds);
    void drawCaret(sf::RenderTarget &target,
                   const ecs::components::UITextInput &input,
                   const ecs::components::Rect &bounds);
    void drawDropdown(sf::RenderTarget &target,
                      const ecs::components::UIDropdown &dropdown,
                      const ecs::components::Rect &bounds);
    void drawDropdownOptions(sf::RenderTarget &target,
                             const ecs::components::UIDropdown &dropdown,
                             const ecs::components::Rect &bounds);
    void drawScrollList(sf::RenderTarget &target,
                        const ecs::components::UIScrollList &list,
                        const ecs::components::Rect &bounds);
    void drawLabel(sf::RenderTarget &target, const std::string &text,
                   const ecs::components::UIWidgetStyle &style,
                   const ecs::components::Rect &bounds, sf::Color color);

    resources::ResourceManager &_resources;
    float _textScale = 1.0f;
};

} // namespace engine::render

#endif /* !UIRENDERSYSTEM_HPP_ */
