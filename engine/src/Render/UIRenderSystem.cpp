/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIRenderSystem.cpp
*/

#include "Engine/Render/UIRenderSystem.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/String.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <utility>

#include "Engine/Ecs/Components/UIButton.hpp"
#include "Engine/Ecs/Components/UIGraph.hpp"
#include "Engine/Ecs/Components/UIShape.hpp"
#include "Engine/Ecs/Components/UISlider.hpp"
#include "Engine/Ecs/Components/UIToggle.hpp"
#include "Engine/Ecs/IndexedZipper.hpp"
#include "Engine/Ecs/Systems/UIWidgetLogic.hpp"
#include "Engine/Ecs/Zipper.hpp"

namespace engine::render {

namespace {

using ecs::Entity;
using ecs::Registry;
using ecs::components::Rect;
using ecs::components::UIButton;
using ecs::components::UIColor;
using ecs::components::UIColorRole;
using ecs::components::UIDropdown;
using ecs::components::UIPanel;
using ecs::components::UIProgressBar;
using ecs::components::UIScrollList;
using ecs::components::UIShape;
using ecs::components::UIShapeKind;
using ecs::components::UISlider;
using ecs::components::UITextAlign;
using ecs::components::UITextInput;
using ecs::components::UITheme;
using ecs::components::UIToggle;
using ecs::components::UIWidgetStyle;
namespace widgets = ecs::systems::widgets;

constexpr float PLATE_PADDING = 3.0f;
constexpr float CROSS_THICKNESS = 0.3f;
constexpr UIColor GAUGE_OUTLINE_COLOR{255, 255, 255, 255};
constexpr UIColor PLATE_COLOR{0, 0, 0, 255};
constexpr float TEXT_PADDING = 6.0f;
constexpr float CARET_WIDTH = 2.0f;

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

UIColor buttonColor(const UITheme &theme, const UIButton &button) noexcept
{
    switch (button.state) {
        case ecs::components::UIButtonState::HOVERED:
            return theme.resolve(UIColorRole::BUTTON_HOVERED,
                                 button.hoveredColor);
        case ecs::components::UIButtonState::PRESSED:
            return theme.resolve(UIColorRole::BUTTON_PRESSED,
                                 button.pressedColor);
        case ecs::components::UIButtonState::DISABLED:
            return theme.resolve(UIColorRole::BUTTON_DISABLED,
                                 button.disabledColor);
        default:
            return theme.resolve(UIColorRole::BUTTON, button.normalColor);
    }
}

void drawRect(sf::RenderTarget &target, const Rect &bounds, sf::Color color)
{
    sf::RectangleShape shape({bounds.width, bounds.height});

    shape.setPosition({bounds.x, bounds.y});
    shape.setFillColor(color);
    target.draw(shape);
}

void drawFrame(sf::RenderTarget &target, const Rect &bounds,
               const UIColor &color, float thickness)
{
    sf::RectangleShape outline({bounds.width, bounds.height});

    outline.setPosition({bounds.x, bounds.y});
    outline.setFillColor(sf::Color::Transparent);
    outline.setOutlineThickness(thickness);
    outline.setOutlineColor(toSfColor(color));
    target.draw(outline);
}

void drawProgressBar(sf::RenderTarget &target, const UIProgressBar &bar,
                     const Rect &bounds, const UITheme &theme)
{
    const Rect fill = UIRenderSystem::fillBounds(bar, bounds);

    drawRect(target, bounds,
             toSfColor(theme.resolve(bar.backgroundRole, bar.backgroundColor)));
    if (fill.width > 0.0f && fill.height > 0.0f)
        drawRect(target, fill,
                 toSfColor(theme.resolve(bar.fillRole, bar.fillColor)));
    if (theme.options().gaugeOutline > 0.0f)
        drawFrame(target, bounds,
                  theme.resolve(UIColorRole::TEXT, GAUGE_OUTLINE_COLOR),
                  theme.options().gaugeOutline);
}

void drawCross(sf::RenderTarget &target, const Rect &bounds, sf::Color color)
{
    const float barWidth = bounds.width * CROSS_THICKNESS;
    const float barHeight = bounds.height * CROSS_THICKNESS;

    drawRect(target,
             {bounds.x, bounds.y + (bounds.height - barHeight) / 2.0f,
              bounds.width, barHeight},
             color);
    drawRect(target,
             {bounds.x + (bounds.width - barWidth) / 2.0f, bounds.y, barWidth,
              bounds.height},
             color);
}

void drawTriangle(sf::RenderTarget &target, const Rect &bounds, sf::Color color)
{
    sf::ConvexShape triangle(3);

    triangle.setPoint(0, {bounds.x + bounds.width / 2.0f, bounds.y});
    triangle.setPoint(1, {bounds.x + bounds.width, bounds.y + bounds.height});
    triangle.setPoint(2, {bounds.x, bounds.y + bounds.height});
    triangle.setFillColor(color);
    target.draw(triangle);
}

void drawCircle(sf::RenderTarget &target, const Rect &bounds, sf::Color color)
{
    sf::CircleShape circle(std::min(bounds.width, bounds.height) / 2.0f);

    circle.setPosition({bounds.x, bounds.y});
    circle.setFillColor(color);
    target.draw(circle);
}

void appendBar(sf::VertexArray &vertices, const Rect &bar, sf::Color color)
{
    const sf::Vector2f topLeft{bar.x, bar.y};
    const sf::Vector2f topRight{bar.x + bar.width, bar.y};
    const sf::Vector2f bottomRight{bar.x + bar.width, bar.y + bar.height};
    const sf::Vector2f bottomLeft{bar.x, bar.y + bar.height};

    for (const sf::Vector2f &corner :
         {topLeft, topRight, bottomRight, topLeft, bottomRight, bottomLeft})
        vertices.append(sf::Vertex{corner, color});
}

float graphTop(const ecs::components::UIGraph &graph) noexcept
{
    if (graph.maxValue > 0.0f)
        return graph.maxValue;
    const auto largest =
        std::max_element(graph.samples.begin(), graph.samples.end());

    return largest == graph.samples.end() ? 0.0f : *largest;
}

sf::VertexArray graphBars(const ecs::components::UIGraph &graph,
                          const Rect &bounds, sf::Color color)
{
    const float top = graphTop(graph);
    const std::size_t slots = std::max(graph.capacity, graph.samples.size());
    const float barWidth =
        bounds.width / static_cast<float>(std::max<std::size_t>(slots, 1));
    const float firstX = bounds.x + bounds.width -
                         barWidth * static_cast<float>(graph.samples.size());
    sf::VertexArray bars(sf::PrimitiveType::Triangles);

    if (top <= 0.0f)
        return bars;
    for (std::size_t i = 0; i < graph.samples.size(); ++i) {
        const float ratio = std::clamp(graph.samples[i] / top, 0.0f, 1.0f);
        const float height = bounds.height * ratio;
        appendBar(bars,
                  {firstX + barWidth * static_cast<float>(i),
                   bounds.y + bounds.height - height, barWidth, height},
                  color);
    }
    return bars;
}

void drawGraph(sf::RenderTarget &target, const ecs::components::UIGraph &graph,
               const Rect &bounds, const UITheme &theme)
{
    drawRect(
        target, bounds,
        toSfColor(theme.resolve(graph.backgroundRole, graph.backgroundColor)));
    target.draw(
        graphBars(graph, bounds,
                  toSfColor(theme.resolve(graph.barRole, graph.barColor))));
}

void drawShape(sf::RenderTarget &target, const UIShape &shape,
               const Rect &bounds, const UITheme &theme)
{
    const sf::Color color =
        toSfColor(theme.resolve(shape.colorRole, shape.color));

    switch (shape.kind) {
        case UIShapeKind::CIRCLE:
            drawCircle(target, bounds, color);
            break;
        case UIShapeKind::TRIANGLE:
            drawTriangle(target, bounds, color);
            break;
        case UIShapeKind::CROSS:
            drawCross(target, bounds, color);
            break;
        default:
            drawRect(target, bounds, color);
            break;
    }
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

template <typename... Components>
bool hasAny(const Registry &registry, Entity entity)
{
    return ((registry.getComponents<Components>().get(entity) != nullptr) ||
            ...);
}

bool isDrawable(const Registry &registry, Entity entity)
{
    return hasAny<UIPanel, UIProgressBar, ecs::components::UIText, UIButton,
                  UITextInput, UISlider, UIToggle, UIDropdown, UIScrollList,
                  UIShape, ecs::components::UIGraph>(registry, entity);
}

sf::Color dimmed(const UIColor &color) noexcept
{
    return {color.r, color.g, color.b, static_cast<std::uint8_t>(color.a / 2)};
}

void drawSlider(sf::RenderTarget &target, const UISlider &slider,
                const UIWidgetStyle &style, const Rect &bounds)
{
    const float ratio = widgets::sliderRatio(slider);
    const Rect track{bounds.x, bounds.y + bounds.height / 2.0f - 2.0f,
                     bounds.width, 4.0f};
    const Rect handle{bounds.x + bounds.width * ratio - 4.0f,
                      bounds.y + bounds.height * 0.2f, 8.0f,
                      bounds.height * 0.6f};

    drawRect(target, track, toSfColor(style.background));
    drawRect(target, {track.x, track.y, track.width * ratio, track.height},
             toSfColor(style.accent));
    drawRect(target, handle, toSfColor(style.foreground));
}

Rect toggleBox(const Rect &bounds) noexcept
{
    const float side = std::min(bounds.width, bounds.height);

    return {bounds.x, bounds.y + (bounds.height - side) / 2.0f, side, side};
}

void drawToggle(sf::RenderTarget &target, const UIToggle &toggle,
                const UIWidgetStyle &style, const Rect &bounds)
{
    const Rect box = toggleBox(bounds);
    const float inset = box.width * 0.25f;

    drawRect(target, box, toSfColor(style.background));
    if (toggle.checked)
        drawRect(target,
                 {box.x + inset, box.y + inset, box.width - 2 * inset,
                  box.height - 2 * inset},
                 toSfColor(style.accent));
}

void drawArrow(sf::RenderTarget &target, const Rect &bounds, sf::Color color)
{
    const float size = bounds.height * 0.3f;
    const sf::Vector2f center{bounds.x + bounds.width - bounds.height / 2.0f,
                              bounds.y + bounds.height / 2.0f};
    sf::ConvexShape arrow(3);

    arrow.setPoint(0, {center.x - size, center.y - size / 2.0f});
    arrow.setPoint(1, {center.x + size, center.y - size / 2.0f});
    arrow.setPoint(2, {center.x, center.y + size / 2.0f});
    arrow.setFillColor(color);
    target.draw(arrow);
}

Rect padded(const Rect &bounds) noexcept
{
    return {bounds.x + TEXT_PADDING, bounds.y,
            std::max(0.0f, bounds.width - 2 * TEXT_PADDING), bounds.height};
}

} // namespace

UIRenderSystem::UIRenderSystem(resources::ResourceManager &resources) noexcept
    : _resources(resources)
{}

void UIRenderSystem::setTheme(const UITheme &theme) noexcept
{
    _theme = &theme;
}

const UITheme &UIRenderSystem::theme() const noexcept
{
    return _theme != nullptr ? *_theme : _noTheme;
}

void UIRenderSystem::render(sf::RenderTarget &target,
                            const ecs::Registry &registry)
{
    const sf::View previous = target.getView();

    target.setView(screenView(target.getSize()));
    for (const Entity entity : drawOrder(registry))
        drawEntity(target, registry, entity);
    drawOpenDropdowns(target, registry);
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

    _textScale = registry.getComponents<ecs::components::UITransform>()
                     .get(entity)
                     ->resolvedScale;
    drawBackground(target, registry, entity, bounds);
    if (const auto *bar = registry.getComponents<UIProgressBar>().get(entity))
        drawProgressBar(target, *bar, bounds, theme());
    if (const auto *shape = registry.getComponents<UIShape>().get(entity))
        drawShape(target, *shape, bounds, theme());
    if (const auto *graph =
            registry.getComponents<ecs::components::UIGraph>().get(entity))
        drawGraph(target, *graph, bounds, theme());
    if (const auto *text =
            registry.getComponents<ecs::components::UIText>().get(entity))
        drawText(target, *text, bounds);
    drawWidgets(target, registry, entity, bounds);
}

void UIRenderSystem::drawBackground(sf::RenderTarget &target,
                                    const ecs::Registry &registry,
                                    ecs::Entity entity, const Rect &bounds)
{
    if (const auto *panel = registry.getComponents<UIPanel>().get(entity))
        drawPanel(target, *panel, bounds);
    else if (const auto *button =
                 registry.getComponents<UIButton>().get(entity))
        drawRect(target, bounds, toSfColor(buttonColor(theme(), *button)));
}

void UIRenderSystem::drawPanel(sf::RenderTarget &target,
                               const ecs::components::UIPanel &panel,
                               const ecs::components::Rect &bounds)
{
    const sf::Texture *texture = _resources.texture(panel.textureId);
    sf::RenderStates states;

    if (texture == nullptr) {
        drawRect(target, bounds,
                 toSfColor(theme().resolve(panel.colorRole, panel.color)));
        return;
    }
    states.texture = texture;
    target.draw(panelVertices(panel, bounds, texture->getSize()), states);
}

unsigned int UIRenderSystem::scaledTextSize(unsigned int size) const noexcept
{
    return static_cast<unsigned int>(
        std::lround(static_cast<float>(size) * _textScale));
}

void UIRenderSystem::drawPlate(sf::RenderTarget &target,
                               const sf::Text &label) const
{
    const sf::FloatRect area = label.getGlobalBounds();

    drawRect(target,
             {area.position.x - PLATE_PADDING, area.position.y - PLATE_PADDING,
              area.size.x + 2 * PLATE_PADDING, area.size.y + 2 * PLATE_PADDING},
             toSfColor(theme().resolve(UIColorRole::BACKGROUND, PLATE_COLOR)));
}

void UIRenderSystem::drawText(sf::RenderTarget &target,
                              const ecs::components::UIText &text,
                              const ecs::components::Rect &bounds)
{
    const sf::Font *font = _resources.font(text.fontId);
    const unsigned int size = scaledTextSize(text.characterSize);

    if (font == nullptr || text.text.empty())
        return;
    sf::Text label(
        *font, sf::String::fromUtf8(text.text.begin(), text.text.end()), size);
    const sf::FloatRect local = label.getLocalBounds();
    label.setOrigin({local.position.x + local.size.x * alignFactor(text.align),
                     font->getLineSpacing(size) / 2.0f});
    label.setPosition(textAnchor(text.align, bounds));
    label.setFillColor(toSfColor(theme().resolve(text.colorRole, text.color)));
    if (theme().hasPlate(text.colorRole))
        drawPlate(target, label);
    target.draw(label);
}

void UIRenderSystem::drawWidgets(sf::RenderTarget &target,
                                 const ecs::Registry &registry,
                                 ecs::Entity entity, const Rect &bounds)
{
    if (const auto *input = registry.getComponents<UITextInput>().get(entity))
        drawTextInput(target, *input, bounds);
    if (const auto *slider = registry.getComponents<UISlider>().get(entity))
        drawSliderWidget(target, *slider, bounds);
    if (const auto *toggle = registry.getComponents<UIToggle>().get(entity))
        drawToggleWidget(target, *toggle, bounds);
    if (const auto *dropdown = registry.getComponents<UIDropdown>().get(entity))
        drawDropdown(target, *dropdown, bounds);
    if (const auto *list = registry.getComponents<UIScrollList>().get(entity))
        drawScrollList(target, *list, bounds);
    const auto *button = registry.getComponents<UIButton>().get(entity);
    if (button != nullptr && button->isFocused)
        drawFocusOutline(
            target, bounds,
            theme().resolve(UIColorRole::FOCUS, button->focusedOutlineColor));
}

void UIRenderSystem::drawFocusOutline(sf::RenderTarget &target,
                                      const Rect &bounds,
                                      const UIColor &color) const
{
    drawFrame(target, bounds, color, theme().options().focusThickness);
}

void UIRenderSystem::drawSliderWidget(sf::RenderTarget &target,
                                      const UISlider &slider,
                                      const Rect &bounds) const
{
    const UIWidgetStyle style = theme().styled(slider.style);

    drawSlider(target, slider, style, bounds);
    if (slider.isFocused)
        drawFocusOutline(target, bounds, style.focusOutline);
}

void UIRenderSystem::drawToggleWidget(sf::RenderTarget &target,
                                      const UIToggle &toggle,
                                      const Rect &bounds) const
{
    const UIWidgetStyle style = theme().styled(toggle.style);

    drawToggle(target, toggle, style, bounds);
    if (toggle.isFocused)
        drawFocusOutline(target, toggleBox(bounds), style.focusOutline);
}

void UIRenderSystem::drawOpenDropdowns(sf::RenderTarget &target,
                                       const ecs::Registry &registry)
{
    for (auto [transform, dropdown] :
         ecs::makeZipper(registry.getComponents<ecs::components::UITransform>(),
                         registry.getComponents<UIDropdown>())) {
        if (!dropdown.open)
            continue;
        _textScale = transform.resolvedScale;
        drawDropdownOptions(target, dropdown, transform.actualBounds);
    }
}

void UIRenderSystem::drawTextInput(sf::RenderTarget &target,
                                   const UITextInput &input, const Rect &bounds)
{
    const UIWidgetStyle style = theme().styled(input.style);
    const bool placeholder = input.text.empty() && !input.isFocused;

    drawRect(target, bounds, toSfColor(style.background));
    drawLabel(
        target, placeholder ? input.placeholder : input.text, style, bounds,
        placeholder ? dimmed(style.foreground) : toSfColor(style.foreground));
    if (!input.isFocused)
        return;
    drawCaret(target, input, bounds);
    drawFocusOutline(target, bounds, style.focusOutline);
}

void UIRenderSystem::drawCaret(sf::RenderTarget &target,
                               const UITextInput &input, const Rect &bounds)
{
    const sf::Font *font = _resources.font(input.style.fontId);
    float x = padded(bounds).x;

    if (font != nullptr) {
        const sf::Text text(
            *font, sf::String::fromUtf8(input.text.begin(), input.text.end()),
            scaledTextSize(input.style.characterSize));
        x += text.findCharacterPos(
                     widgets::characterIndex(input.text, input.caret))
                 .x;
    }
    drawRect(
        target,
        {x, bounds.y + bounds.height * 0.2f, CARET_WIDTH, bounds.height * 0.6f},
        toSfColor(theme().styled(input.style).foreground));
}

void UIRenderSystem::drawDropdown(sf::RenderTarget &target,
                                  const UIDropdown &dropdown,
                                  const Rect &bounds)
{
    const UIWidgetStyle style = theme().styled(dropdown.style);
    drawRect(target, bounds, toSfColor(style.background));
    if (dropdown.selected < dropdown.options.size())
        drawLabel(target, dropdown.options[dropdown.selected], style, bounds,
                  toSfColor(style.foreground));
    drawArrow(target, bounds, toSfColor(style.foreground));
    if (dropdown.isFocused)
        drawFocusOutline(target, bounds, style.focusOutline);
}

void UIRenderSystem::drawDropdownOptions(sf::RenderTarget &target,
                                         const UIDropdown &dropdown,
                                         const Rect &bounds)
{
    const UIWidgetStyle style = theme().styled(dropdown.style);
    for (std::size_t i = 0; i < dropdown.options.size(); ++i) {
        const Rect row = widgets::dropdownOptionBounds(bounds, i);
        const UIColor &fill =
            i == dropdown.highlighted ? style.accent : style.background;
        drawRect(target, row, toSfColor(fill));
        drawLabel(target, dropdown.options[i], style, row,
                  toSfColor(style.foreground));
    }
}

void UIRenderSystem::drawScrollList(sf::RenderTarget &target,
                                    const UIScrollList &list,
                                    const Rect &bounds)
{
    const UIWidgetStyle style = theme().styled(list.style);
    drawRect(target, bounds, toSfColor(style.background));
    for (std::size_t visible = 0; visible < widgets::visibleRowCount(list);
         ++visible) {
        const std::size_t row = list.scrollOffset + visible;
        if (row >= list.rows.size())
            break;
        const Rect rect = widgets::scrollRowBounds(list, bounds, visible);
        if (list.selected == row)
            drawRect(target, rect, toSfColor(style.accent));
        drawLabel(target, list.rows[row], style, rect,
                  toSfColor(style.foreground));
    }
    if (list.isFocused)
        drawFocusOutline(target, bounds, style.focusOutline);
}

void UIRenderSystem::drawLabel(sf::RenderTarget &target,
                               const std::string &text,
                               const UIWidgetStyle &style, const Rect &bounds,
                               sf::Color color)
{
    const ecs::components::UIText label{text,
                                        style.fontId,
                                        style.characterSize,
                                        {color.r, color.g, color.b, color.a},
                                        UITextAlign::LEFT};

    drawText(target, label, padded(bounds));
}

} // namespace engine::render
