/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIRenderSystem.cpp
*/

#include "Engine/Render/UIRenderSystem.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/String.hpp>
#include <algorithm>
#include <array>
#include <cstddef>
#include <utility>

#include "Engine/Ecs/IndexedZipper.hpp"

namespace engine::render {

namespace {

using ecs::Entity;
using ecs::Registry;
using ecs::components::Rect;
using ecs::components::UIColor;
using ecs::components::UIPanel;
using ecs::components::UIProgressBar;
using ecs::components::UITextAlign;

sf::Color toSfColor(const UIColor &color) noexcept
{
    return {color.r, color.g, color.b, color.a};
}

float alignFactor(UITextAlign align) noexcept
{
    switch (align) {
        case UITextAlign::CENTER:
            return 0.5f;
        case UITextAlign::RIGHT:
            return 1.0f;
        default:
            return 0.0f;
    }
}

void drawRect(sf::RenderTarget &target, const Rect &bounds, sf::Color color)
{
    sf::RectangleShape shape({bounds.width, bounds.height});

    shape.setPosition({bounds.x, bounds.y});
    shape.setFillColor(color);
    target.draw(shape);
}

void drawProgressBar(sf::RenderTarget &target, const UIProgressBar &bar,
                     const Rect &bounds)
{
    const Rect fill = UIRenderSystem::fillBounds(bar, bounds);

    drawRect(target, bounds, toSfColor(bar.backgroundColor));
    if (fill.width > 0.0f && fill.height > 0.0f)
        drawRect(target, fill, toSfColor(bar.fillColor));
}

Rect textureSource(const Rect &textureRect, sf::Vector2u textureSize) noexcept
{
    if (textureRect.width > 0.0f && textureRect.height > 0.0f)
        return textureRect;
    return {0.0f, 0.0f, static_cast<float>(textureSize.x),
            static_cast<float>(textureSize.y)};
}

std::array<float, 4> sliceAxis(float start, float length, float first,
                               float last) noexcept
{
    const float total = first + last;
    const float scale = total > length && total > 0.0f ? length / total : 1.0f;

    return {start, start + first * scale, start + length - last * scale,
            start + length};
}

void appendQuad(sf::VertexArray &vertices, const Rect &screen,
                const Rect &source, sf::Color color)
{
    const float right = screen.x + screen.width;
    const float bottom = screen.y + screen.height;
    const float sourceRight = source.x + source.width;
    const float sourceBottom = source.y + source.height;
    const sf::Vertex topLeft{{screen.x, screen.y}, color, {source.x, source.y}};
    const sf::Vertex topRight{
        {right, screen.y}, color, {sourceRight, source.y}};
    const sf::Vertex bottomRight{
        {right, bottom}, color, {sourceRight, sourceBottom}};
    const sf::Vertex bottomLeft{
        {screen.x, bottom}, color, {source.x, sourceBottom}};

    for (const sf::Vertex &vertex :
         {topLeft, topRight, bottomRight, topLeft, bottomRight, bottomLeft})
        vertices.append(vertex);
}

Rect sliceCell(const std::array<float, 4> &xs, const std::array<float, 4> &ys,
               std::size_t cell) noexcept
{
    const std::size_t col = cell % 3;
    const std::size_t row = cell / 3;

    return {xs[col], ys[row], xs[col + 1] - xs[col], ys[row + 1] - ys[row]};
}

void appendNineSlice(sf::VertexArray &vertices, const UIPanel &panel,
                     const Rect &bounds, const Rect &source)
{
    const auto &borders = panel.borders;
    const auto xs =
        sliceAxis(bounds.x, bounds.width, borders.left, borders.right);
    const auto ys =
        sliceAxis(bounds.y, bounds.height, borders.top, borders.bottom);
    const auto us =
        sliceAxis(source.x, source.width, borders.left, borders.right);
    const auto vs =
        sliceAxis(source.y, source.height, borders.top, borders.bottom);

    for (std::size_t cell = 0; cell < 9; ++cell)
        appendQuad(vertices, sliceCell(xs, ys, cell), sliceCell(us, vs, cell),
                   toSfColor(panel.color));
}

bool isDrawable(const Registry &registry, Entity entity)
{
    return registry.getComponents<UIPanel>().get(entity) != nullptr ||
           registry.getComponents<UIProgressBar>().get(entity) != nullptr ||
           registry.getComponents<ecs::components::UIText>().get(entity) !=
               nullptr;
}

} // namespace

UIRenderSystem::UIRenderSystem(resources::ResourceManager &resources) noexcept
    : _resources(resources)
{}

void UIRenderSystem::render(sf::RenderTarget &target,
                            const ecs::Registry &registry)
{
    const sf::View previous = target.getView();

    target.setView(screenView(target.getSize()));
    for (const Entity entity : drawOrder(registry))
        drawEntity(target, registry, entity);
    target.setView(previous);
}

sf::View UIRenderSystem::screenView(sf::Vector2u targetSize)
{
    return sf::View(sf::FloatRect({0.0f, 0.0f}, sf::Vector2f(targetSize)));
}

std::vector<ecs::Entity>
UIRenderSystem::drawOrder(const ecs::Registry &registry)
{
    std::vector<std::pair<int, Entity>> layers;
    std::vector<Entity> order;

    for (auto [index, transform] : ecs::makeIndexedZipper(
             registry.getComponents<ecs::components::UITransform>())) {
        const Entity entity = registry.entityAt(index);
        if (isDrawable(registry, entity))
            layers.emplace_back(transform.zIndex, entity);
    }
    std::stable_sort(
        layers.begin(), layers.end(),
        [](const auto &lhs, const auto &rhs) { return lhs.first < rhs.first; });
    order.reserve(layers.size());
    for (const auto &layer : layers)
        order.push_back(layer.second);
    return order;
}

float UIRenderSystem::fillRatio(
    const ecs::components::UIProgressBar &bar) noexcept
{
    if (bar.max <= bar.min)
        return 0.0f;
    return std::clamp((bar.value - bar.min) / (bar.max - bar.min), 0.0f, 1.0f);
}

ecs::components::Rect
UIRenderSystem::fillBounds(const ecs::components::UIProgressBar &bar,
                           const ecs::components::Rect &bounds) noexcept
{
    const float ratio = fillRatio(bar);

    if (bar.direction == ecs::components::UIFillDirection::VERTICAL) {
        const float height = bounds.height * ratio;
        return {bounds.x, bounds.y + bounds.height - height, bounds.width,
                height};
    }
    return {bounds.x, bounds.y, bounds.width * ratio, bounds.height};
}

sf::VertexArray
UIRenderSystem::panelVertices(const ecs::components::UIPanel &panel,
                              const ecs::components::Rect &bounds,
                              sf::Vector2u textureSize)
{
    const Rect source = textureSource(panel.textureRect, textureSize);
    sf::VertexArray vertices(sf::PrimitiveType::Triangles);

    if (panel.mode == ecs::components::UIPanelMode::NINE_SLICE)
        appendNineSlice(vertices, panel, bounds, source);
    else
        appendQuad(vertices, bounds, source, toSfColor(panel.color));
    return vertices;
}

sf::Vector2f
UIRenderSystem::textAnchor(ecs::components::UITextAlign align,
                           const ecs::components::Rect &bounds) noexcept
{
    return {bounds.x + bounds.width * alignFactor(align),
            bounds.y + bounds.height / 2.0f};
}

void UIRenderSystem::drawEntity(sf::RenderTarget &target,
                                const ecs::Registry &registry,
                                ecs::Entity entity)
{
    const Rect &bounds = registry.getComponents<ecs::components::UITransform>()
                             .get(entity)
                             ->actualBounds;

    if (const auto *panel = registry.getComponents<UIPanel>().get(entity))
        drawPanel(target, *panel, bounds);
    if (const auto *bar = registry.getComponents<UIProgressBar>().get(entity))
        drawProgressBar(target, *bar, bounds);
    if (const auto *text =
            registry.getComponents<ecs::components::UIText>().get(entity))
        drawText(target, *text, bounds);
}

void UIRenderSystem::drawPanel(sf::RenderTarget &target,
                               const ecs::components::UIPanel &panel,
                               const ecs::components::Rect &bounds)
{
    const sf::Texture *texture = _resources.texture(panel.textureId);
    sf::RenderStates states;

    if (texture == nullptr) {
        drawRect(target, bounds, toSfColor(panel.color));
        return;
    }
    states.texture = texture;
    target.draw(panelVertices(panel, bounds, texture->getSize()), states);
}

void UIRenderSystem::drawText(sf::RenderTarget &target,
                              const ecs::components::UIText &text,
                              const ecs::components::Rect &bounds)
{
    const sf::Font *font = _resources.font(text.fontId);

    if (font == nullptr || text.text.empty())
        return;
    sf::Text label(*font,
                   sf::String::fromUtf8(text.text.begin(), text.text.end()),
                   text.characterSize);
    const sf::FloatRect local = label.getLocalBounds();
    label.setOrigin({local.position.x + local.size.x * alignFactor(text.align),
                     font->getLineSpacing(text.characterSize) / 2.0f});
    label.setPosition(textAnchor(text.align, bounds));
    label.setFillColor(toSfColor(text.color));
    target.draw(label);
}

} // namespace engine::render
