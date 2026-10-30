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

namespace engine::render {

namespace {

using ecs::SparseArray;
using ecs::components::Rect;
using ecs::components::UIColor;
using ecs::components::UIPanel;
using ecs::components::UIProgressBar;
using ecs::components::UITextAlign;

template <typename Component>
const Component *componentAt(const SparseArray<Component> &array,
                             std::size_t index) noexcept
{
    if (index >= array.size())
        return nullptr;
    const auto &slot = *(array.begin() + static_cast<std::ptrdiff_t>(index));
    return slot.has_value() ? &*slot : nullptr;
}

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

bool isDrawable(std::size_t index, const SparseArray<UIPanel> &panels,
                const SparseArray<UIProgressBar> &bars,
                const SparseArray<ecs::components::UIText> &texts) noexcept
{
    return componentAt(panels, index) != nullptr ||
           componentAt(bars, index) != nullptr ||
           componentAt(texts, index) != nullptr;
}

} // namespace

UIRenderSystem::UIRenderSystem(resources::ResourceManager &resources) noexcept
    : _resources(resources)
{}

void UIRenderSystem::render(
    sf::RenderTarget &target,
    const ecs::SparseArray<ecs::components::UITransform> &transforms,
    const ecs::SparseArray<ecs::components::UIPanel> &panels,
    const ecs::SparseArray<ecs::components::UIProgressBar> &bars,
    const ecs::SparseArray<ecs::components::UIText> &texts)
{
    const sf::View previous = target.getView();

    target.setView(screenView(target.getSize()));
    for (const std::size_t index : drawOrder(transforms, panels, bars, texts)) {
        const Rect &bounds = componentAt(transforms, index)->actualBounds;
        if (const auto *panel = componentAt(panels, index))
            drawPanel(target, *panel, bounds);
        if (const auto *bar = componentAt(bars, index))
            drawProgressBar(target, *bar, bounds);
        if (const auto *text = componentAt(texts, index))
            drawText(target, *text, bounds);
    }
    target.setView(previous);
}

sf::View UIRenderSystem::screenView(sf::Vector2u targetSize)
{
    return sf::View(sf::FloatRect({0.0f, 0.0f}, sf::Vector2f(targetSize)));
}

std::vector<std::size_t> UIRenderSystem::drawOrder(
    const ecs::SparseArray<ecs::components::UITransform> &transforms,
    const ecs::SparseArray<ecs::components::UIPanel> &panels,
    const ecs::SparseArray<ecs::components::UIProgressBar> &bars,
    const ecs::SparseArray<ecs::components::UIText> &texts)
{
    std::vector<std::size_t> order;

    for (std::size_t i = 0; i < transforms.size(); ++i) {
        if (componentAt(transforms, i) != nullptr &&
            isDrawable(i, panels, bars, texts))
            order.push_back(i);
    }
    std::stable_sort(order.begin(), order.end(),
                     [&transforms](std::size_t lhs, std::size_t rhs) {
                         return componentAt(transforms, lhs)->zIndex <
                                componentAt(transforms, rhs)->zIndex;
                     });
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
