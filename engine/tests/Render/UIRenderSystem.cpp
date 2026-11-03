/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** UIRenderSystem.cpp
*/

#include "Engine/Render/UIRenderSystem.hpp"

#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <array>
#include <catch2/catch_test_macros.hpp>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

#include "Engine/Ecs/Components/UIComponents.hpp"
#include "Engine/Ecs/Components/UIShape.hpp"
#include "Engine/Ecs/Registry.hpp"

using engine::ecs::Entity;
using engine::ecs::Registry;
using engine::ecs::components::Rect;
using engine::ecs::components::UIBorders;
using engine::ecs::components::UIColor;
using engine::ecs::components::UIDropdown;
using engine::ecs::components::UIFillDirection;
using engine::ecs::components::UIPanel;
using engine::ecs::components::UIPanelMode;
using engine::ecs::components::UIProgressBar;
using engine::ecs::components::UIScrollList;
using engine::ecs::components::UISlider;
using engine::ecs::components::UIText;
using engine::ecs::components::UITextAlign;
using engine::ecs::components::UITextInput;
using engine::ecs::components::UIToggle;
using engine::ecs::components::UITransform;
using engine::ecs::components::UIWidgetStyle;
using engine::render::UIRenderSystem;
using engine::resources::ResourceManager;

namespace {

struct UIWorld {
    Registry registry;

    UIWorld()
    {
        engine::ecs::registerUIComponents(registry);
    }

    Entity spawn(const Rect &bounds, int zIndex = 0)
    {
        const Entity entity = registry.spawnEntity();
        UITransform transform;

        transform.zIndex = zIndex;
        transform.actualBounds = bounds;
        registry.addComponent(entity, transform);
        return entity;
    }

    template <typename Component>
    void add(Entity entity, Component component)
    {
        registry.addComponent(entity, std::move(component));
    }
};

UIPanel colorPanel(UIColor color)
{
    UIPanel panel;

    panel.color = color;
    return panel;
}

UIWidgetStyle opaqueStyle()
{
    UIWidgetStyle style;

    style.background = {0, 0, 255, 255};
    style.foreground = {255, 255, 255, 255};
    style.accent = {0, 255, 0, 255};
    style.focusOutline = {255, 255, 0, 255};
    return style;
}

sf::Image renderWorld(UIWorld &world, sf::Vector2u size)
{
    sf::RenderTexture target;

    if (!target.resize(size))
        return {};
    ResourceManager resources("does/not/exist");
    UIRenderSystem system(resources);
    target.clear(sf::Color::Black);
    system.render(target, world.registry);
    target.display();
    return target.getTexture().copyToImage();
}

bool hasDisplay()
{
#if defined(__linux__)
    const char *display = std::getenv("DISPLAY");
    return display != nullptr && *display != '\0';
#else
    return true;
#endif
}

std::filesystem::path findSystemFont()
{
    const std::array<std::filesystem::path, 3> candidates{
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf", "C:/Windows/Fonts/arial.ttf"};

    for (const auto &candidate : candidates) {
        if (std::filesystem::exists(candidate))
            return candidate;
    }
    return {};
}

} // namespace

TEST_CASE("fill ratio is clamped to [0, 1]", "[render][ui][progress]")
{
    UIProgressBar bar;
    bar.min = 0.0f;
    bar.max = 200.0f;

    bar.value = 50.0f;
    REQUIRE(UIRenderSystem::fillRatio(bar) == 0.25f);
    bar.value = -10.0f;
    REQUIRE(UIRenderSystem::fillRatio(bar) == 0.0f);
    bar.value = 500.0f;
    REQUIRE(UIRenderSystem::fillRatio(bar) == 1.0f);
}

TEST_CASE("an empty range draws an empty bar", "[render][ui][progress]")
{
    UIProgressBar bar;
    bar.min = 5.0f;
    bar.max = 5.0f;
    bar.value = 5.0f;

    REQUIRE(UIRenderSystem::fillRatio(bar) == 0.0f);
}

TEST_CASE("horizontal bars fill from the left", "[render][ui][progress]")
{
    UIProgressBar bar;
    bar.value = 0.5f;

    const Rect fill =
        UIRenderSystem::fillBounds(bar, Rect{10.0f, 20.0f, 200.0f, 30.0f});

    REQUIRE(fill == Rect{10.0f, 20.0f, 100.0f, 30.0f});
}

TEST_CASE("vertical bars fill from the bottom", "[render][ui][progress]")
{
    UIProgressBar bar;
    bar.value = 0.25f;
    bar.direction = UIFillDirection::VERTICAL;

    const Rect fill =
        UIRenderSystem::fillBounds(bar, Rect{0.0f, 100.0f, 20.0f, 80.0f});

    REQUIRE(fill == Rect{0.0f, 160.0f, 20.0f, 20.0f});
}

TEST_CASE("text anchor follows the alignment", "[render][ui][text]")
{
    const Rect bounds{100.0f, 50.0f, 200.0f, 40.0f};

    REQUIRE(UIRenderSystem::textAnchor(UITextAlign::LEFT, bounds) ==
            sf::Vector2f{100.0f, 70.0f});
    REQUIRE(UIRenderSystem::textAnchor(UITextAlign::CENTER, bounds) ==
            sf::Vector2f{200.0f, 70.0f});
    REQUIRE(UIRenderSystem::textAnchor(UITextAlign::RIGHT, bounds) ==
            sf::Vector2f{300.0f, 70.0f});
}

TEST_CASE("the UI view maps one unit to one pixel", "[render][ui][view]")
{
    const sf::View view = UIRenderSystem::screenView({1920, 1080});

    REQUIRE(view.getSize() == sf::Vector2f{1920.0f, 1080.0f});
    REQUIRE(view.getCenter() == sf::Vector2f{960.0f, 540.0f});
}

TEST_CASE("draw order follows zIndex and skips non drawable entities",
          "[render][ui][order]")
{
    UIWorld world;
    const Entity top = world.spawn({}, 5);
    const Entity bottom = world.spawn({}, -1);
    const Entity transformOnly = world.spawn({}, 0);
    const Entity sameLayer = world.spawn({}, 5);
    const Entity noTransform = world.registry.spawnEntity();

    world.add(top, UIPanel{});
    world.add(bottom, UIProgressBar{});
    world.add(sameLayer, UIText{});
    world.add(noTransform, UIPanel{});

    const std::vector<Entity> order = UIRenderSystem::drawOrder(world.registry);

    REQUIRE(order == std::vector<Entity>{bottom, top, sameLayer});
    REQUIRE(transformOnly != top);
}

TEST_CASE("a stretched panel is one textured quad", "[render][ui][panel]")
{
    UIPanel panel;
    panel.textureId = "panel.png";

    const sf::VertexArray vertices = UIRenderSystem::panelVertices(
        panel, Rect{10.0f, 20.0f, 100.0f, 50.0f}, {64, 32});

    REQUIRE(vertices.getVertexCount() == 6);
    REQUIRE(vertices[0].position == sf::Vector2f{10.0f, 20.0f});
    REQUIRE(vertices[2].position == sf::Vector2f{110.0f, 70.0f});
    REQUIRE(vertices[0].texCoords == sf::Vector2f{0.0f, 0.0f});
    REQUIRE(vertices[2].texCoords == sf::Vector2f{64.0f, 32.0f});
}

TEST_CASE("a 9-slice panel keeps its corners unscaled", "[render][ui][panel]")
{
    UIPanel panel;
    panel.textureId = "panel.png";
    panel.mode = UIPanelMode::NINE_SLICE;
    panel.textureRect = Rect{0.0f, 0.0f, 30.0f, 30.0f};
    panel.borders = UIBorders{10.0f, 10.0f, 10.0f, 10.0f};

    const sf::VertexArray vertices = UIRenderSystem::panelVertices(
        panel, Rect{0.0f, 0.0f, 200.0f, 100.0f}, {64, 64});

    REQUIRE(vertices.getVertexCount() == 54);
    REQUIRE(vertices[2].position == sf::Vector2f{10.0f, 10.0f});
    REQUIRE(vertices[2].texCoords == sf::Vector2f{10.0f, 10.0f});
    REQUIRE(vertices[4 * 6 + 2].position == sf::Vector2f{190.0f, 90.0f});
    REQUIRE(vertices[4 * 6 + 2].texCoords == sf::Vector2f{20.0f, 20.0f});
    REQUIRE(vertices[53].position == sf::Vector2f{190.0f, 100.0f});
}

TEST_CASE("9-slice borders shrink when the panel is too small",
          "[render][ui][panel]")
{
    UIPanel panel;
    panel.mode = UIPanelMode::NINE_SLICE;
    panel.textureRect = Rect{0.0f, 0.0f, 30.0f, 30.0f};
    panel.borders = UIBorders{10.0f, 10.0f, 10.0f, 10.0f};

    const sf::VertexArray vertices = UIRenderSystem::panelVertices(
        panel, Rect{0.0f, 0.0f, 10.0f, 10.0f}, {64, 64});

    REQUIRE(vertices[2].position == sf::Vector2f{5.0f, 5.0f});
    REQUIRE(vertices[2].texCoords == sf::Vector2f{10.0f, 10.0f});
}

TEST_CASE("UI stays in screen space whatever the world view",
          "[render][ui][gl]")
{
    if (!hasDisplay())
        SKIP("no display to create an OpenGL context");
    sf::RenderTexture target;
    if (!target.resize({64, 64}))
        SKIP("cannot create a render texture");
    sf::View worldView = target.getDefaultView();
    worldView.move({1000.0f, 500.0f});
    target.setView(worldView);
    UIWorld world;
    world.add(world.spawn({8.0f, 8.0f, 16.0f, 16.0f}),
              colorPanel({255, 0, 0, 255}));
    ResourceManager resources;
    UIRenderSystem system(resources);

    target.clear(sf::Color::Black);
    system.render(target, world.registry);
    target.display();
    const sf::Image image = target.getTexture().copyToImage();

    REQUIRE(image.getPixel({16, 16}) == sf::Color::Red);
    REQUIRE(image.getPixel({40, 40}) == sf::Color::Black);
    REQUIRE(target.getView().getCenter() == worldView.getCenter());
}

TEST_CASE("missing fonts and textures do not stop the frame",
          "[render][ui][gl]")
{
    if (!hasDisplay())
        SKIP("no display to create an OpenGL context");
    sf::RenderTexture target;
    if (!target.resize({32, 32}))
        SKIP("cannot create a render texture");
    UIWorld world;
    const Entity entity = world.spawn({0.0f, 0.0f, 32.0f, 32.0f});
    UIPanel panel = colorPanel({0, 255, 0, 255});
    panel.textureId = "missing.png";
    world.add(entity, panel);
    world.add(entity, UIText{"Score", "missing.ttf"});
    ResourceManager resources("does/not/exist");
    UIRenderSystem system(resources);

    target.clear(sf::Color::Black);
    system.render(target, world.registry);
    target.display();

    REQUIRE(target.getTexture().copyToImage().getPixel({16, 16}) ==
            sf::Color::Green);
}

TEST_CASE("progress bars draw background then fill", "[render][ui][gl]")
{
    if (!hasDisplay())
        SKIP("no display to create an OpenGL context");
    sf::RenderTexture target;
    if (!target.resize({40, 10}))
        SKIP("cannot create a render texture");
    UIWorld world;
    UIProgressBar bar;
    bar.value = 0.5f;
    bar.backgroundColor = {0, 0, 255, 255};
    bar.fillColor = {255, 0, 0, 255};
    world.add(world.spawn({0.0f, 0.0f, 40.0f, 10.0f}), bar);
    ResourceManager resources;
    UIRenderSystem system(resources);

    target.clear(sf::Color::Black);
    system.render(target, world.registry);
    target.display();
    const sf::Image image = target.getTexture().copyToImage();

    REQUIRE(image.getPixel({10, 5}) == sf::Color::Red);
    REQUIRE(image.getPixel({30, 5}) == sf::Color::Blue);
}

TEST_CASE("textures are loaded once and shared", "[render][resources][gl]")
{
    if (!hasDisplay())
        SKIP("no display to create an OpenGL context");
    ResourceManager resources(RTYPE_TEST_ASSETS_DIR);

    const sf::Texture *first = resources.texture("sprites/r-typesheet1.gif");
    const sf::Texture *second = resources.texture("sprites/r-typesheet1.gif");

    REQUIRE(first != nullptr);
    REQUIRE(first == second);
    REQUIRE(first->getSize().x > 0);
}

TEST_CASE("text with a real font draws pixels", "[render][ui][gl]")
{
    const std::filesystem::path font = findSystemFont();
    if (!hasDisplay() || font.empty())
        SKIP("no display or no system font available");
    sf::RenderTexture target;
    if (!target.resize({128, 32}))
        SKIP("cannot create a render texture");
    UIWorld world;
    UIText text{"Score", font.filename().string(), 20};
    text.align = UITextAlign::CENTER;
    world.add(world.spawn({0.0f, 0.0f, 128.0f, 32.0f}), text);
    ResourceManager resources(font.parent_path());
    UIRenderSystem system(resources);

    target.clear(sf::Color::Black);
    system.render(target, world.registry);
    target.display();
    const sf::Image image = target.getTexture().copyToImage();

    REQUIRE(image.getPixel({64, 16}) != sf::Color::Black);
    REQUIRE(image.getPixel({2, 16}) == sf::Color::Black);
}

TEST_CASE("widgets are drawn without crashing on missing fonts",
          "[render][ui][gl][widgets]")
{
    if (!hasDisplay())
        SKIP("no display to create an OpenGL context");
    UIWorld world;
    UITextInput input;
    input.text = "Player";
    input.isFocused = true;
    input.style.fontId = "missing.ttf";
    UIDropdown dropdown;
    dropdown.options = {"a", "b"};
    dropdown.open = true;
    UIScrollList list;
    list.rows = {"one", "two", "three"};
    list.selected = 1;
    world.add(world.spawn({0.0f, 0.0f, 60.0f, 20.0f}), input);
    world.add(world.spawn({0.0f, 30.0f, 60.0f, 20.0f}), UISlider{});
    world.add(world.spawn({0.0f, 60.0f, 20.0f, 20.0f}), UIToggle{});
    world.add(world.spawn({70.0f, 0.0f, 60.0f, 20.0f}), dropdown);
    world.add(world.spawn({70.0f, 60.0f, 60.0f, 60.0f}), list);

    REQUIRE(UIRenderSystem::drawOrder(world.registry).size() == 5);
    REQUIRE(renderWorld(world, {140, 130}).getSize().x == 140);
}

TEST_CASE("a checked toggle shows its mark", "[render][ui][gl][widgets]")
{
    if (!hasDisplay())
        SKIP("no display to create an OpenGL context");
    UIWorld world;
    UIToggle checked;
    checked.checked = true;
    checked.style = opaqueStyle();
    UIToggle unchecked = checked;
    unchecked.checked = false;
    world.add(world.spawn({0.0f, 0.0f, 20.0f, 20.0f}), checked);
    world.add(world.spawn({30.0f, 0.0f, 20.0f, 20.0f}), unchecked);

    const sf::Image image = renderWorld(world, {60, 20});

    REQUIRE(image.getPixel({10, 10}) == sf::Color::Green);
    REQUIRE(image.getPixel({40, 10}) == sf::Color::Blue);
}

TEST_CASE("a slider shows its fill and handle", "[render][ui][gl][widgets]")
{
    if (!hasDisplay())
        SKIP("no display to create an OpenGL context");
    UIWorld world;
    UISlider slider;
    slider.value = 0.5f;
    slider.style = opaqueStyle();
    world.add(world.spawn({0.0f, 0.0f, 100.0f, 20.0f}), slider);

    const sf::Image image = renderWorld(world, {100, 20});

    REQUIRE(image.getPixel({25, 10}) == sf::Color::Green);
    REQUIRE(image.getPixel({75, 10}) == sf::Color::Blue);
    REQUIRE(image.getPixel({50, 6}) == sf::Color::White);
}

TEST_CASE("a button shows the color of its state", "[render][ui][gl][widgets]")
{
    if (!hasDisplay())
        SKIP("no display to create an OpenGL context");
    UIWorld world;
    engine::ecs::UIButton button;
    button.normalColor = {255, 0, 0, 255};
    button.hoveredColor = {0, 255, 0, 255};
    const Entity entity = world.spawn({10.0f, 10.0f, 40.0f, 20.0f});
    world.add(entity, button);
    const Entity panelled = world.spawn({10.0f, 40.0f, 40.0f, 20.0f});
    world.add(panelled, button);
    world.add(panelled, colorPanel({0, 0, 255, 255}));

    REQUIRE(renderWorld(world, {60, 70}).getPixel({30, 20}) == sf::Color::Red);
    world.registry.getComponents<engine::ecs::UIButton>().get(entity)->state =
        engine::ecs::UIButtonState::HOVERED;
    const sf::Image image = renderWorld(world, {60, 70});

    REQUIRE(image.getPixel({30, 20}) == sf::Color::Green);
    REQUIRE(image.getPixel({30, 50}) == sf::Color::Blue);
}

TEST_CASE("the focused widget gets an outline", "[render][ui][gl][widgets]")
{
    if (!hasDisplay())
        SKIP("no display to create an OpenGL context");
    UIWorld world;
    UITextInput focused;
    focused.isFocused = true;
    focused.style = opaqueStyle();
    UITextInput idle = focused;
    idle.isFocused = false;
    world.add(world.spawn({10.0f, 10.0f, 60.0f, 20.0f}), focused);
    world.add(world.spawn({10.0f, 50.0f, 60.0f, 20.0f}), idle);

    const sf::Image image = renderWorld(world, {80, 80});

    REQUIRE(image.getPixel({9, 20}) == sf::Color::Yellow);
    REQUIRE(image.getPixel({9, 60}) == sf::Color::Black);
}

TEST_CASE("an open dropdown is drawn over everything",
          "[render][ui][gl][widgets]")
{
    if (!hasDisplay())
        SKIP("no display to create an OpenGL context");
    UIWorld world;
    UIDropdown dropdown;
    dropdown.options = {"720p", "1080p"};
    dropdown.open = true;
    dropdown.style = opaqueStyle();
    world.add(world.spawn({0.0f, 0.0f, 60.0f, 20.0f}, 0), dropdown);
    world.add(world.spawn({0.0f, 20.0f, 60.0f, 40.0f}, 5),
              colorPanel({255, 0, 0, 255}));

    const sf::Image image = renderWorld(world, {60, 60});

    REQUIRE(image.getPixel({30, 30}) == sf::Color::Green);
    REQUIRE(image.getPixel({30, 50}) == sf::Color::Blue);
}

TEST_CASE("the theme recolors without touching the components",
          "[render][ui][gl][theme]")
{
    if (!hasDisplay())
        SKIP("no display to create an OpenGL context");
    UIWorld world;
    UIPanel themed = colorPanel({255, 0, 0, 255});
    themed.colorRole = engine::ecs::UIColorRole::ACCENT;
    world.add(world.spawn({0.0f, 0.0f, 20.0f, 20.0f}), themed);
    world.add(world.spawn({20.0f, 0.0f, 20.0f, 20.0f}),
              colorPanel({255, 0, 0, 255}));
    engine::ecs::UIButton button;
    button.normalColor = {255, 0, 0, 255};
    world.add(world.spawn({40.0f, 0.0f, 20.0f, 20.0f}), button);
    engine::ecs::UITheme theme;
    theme.setColor(engine::ecs::UIColorRole::ACCENT, {0, 255, 0, 255});
    theme.setColor(engine::ecs::UIColorRole::BUTTON, {0, 0, 255, 255});
    sf::RenderTexture target;
    REQUIRE(target.resize({60, 20}));
    ResourceManager resources("does/not/exist");
    UIRenderSystem system(resources);
    system.setTheme(theme);

    target.clear(sf::Color::Black);
    system.render(target, world.registry);
    target.display();

    const sf::Image image = target.getTexture().copyToImage();
    REQUIRE(image.getPixel({10, 10}) == sf::Color::Green);
    REQUIRE(image.getPixel({30, 10}) == sf::Color::Red);
    REQUIRE(image.getPixel({50, 10}) == sf::Color::Blue);
}

TEST_CASE("each shape is drawn with its own figure", "[render][ui][gl][theme]")
{
    if (!hasDisplay())
        SKIP("no display to create an OpenGL context");
    UIWorld world;
    const auto shape = [](engine::ecs::UIShapeKind kind) {
        engine::ecs::UIShape result;
        result.kind = kind;
        return result;
    };
    world.add(world.spawn({0.0f, 0.0f, 20.0f, 20.0f}),
              shape(engine::ecs::UIShapeKind::SQUARE));
    world.add(world.spawn({20.0f, 0.0f, 20.0f, 20.0f}),
              shape(engine::ecs::UIShapeKind::CIRCLE));
    world.add(world.spawn({40.0f, 0.0f, 20.0f, 20.0f}),
              shape(engine::ecs::UIShapeKind::TRIANGLE));
    world.add(world.spawn({60.0f, 0.0f, 20.0f, 20.0f}),
              shape(engine::ecs::UIShapeKind::CROSS));

    const sf::Image image = renderWorld(world, {80, 20});

    REQUIRE(image.getPixel({1, 1}) == sf::Color::White);
    REQUIRE(image.getPixel({21, 1}) == sf::Color::Black);
    REQUIRE(image.getPixel({30, 10}) == sf::Color::White);
    REQUIRE(image.getPixel({41, 1}) == sf::Color::Black);
    REQUIRE(image.getPixel({50, 15}) == sf::Color::White);
    REQUIRE(image.getPixel({61, 1}) == sf::Color::Black);
    REQUIRE(image.getPixel({70, 1}) == sf::Color::White);
}

TEST_CASE("a high-contrast theme thickens outlines", "[render][ui][gl][theme]")
{
    if (!hasDisplay())
        SKIP("no display to create an OpenGL context");
    UIWorld world;
    engine::ecs::UIButton button;
    button.isFocused = true;
    button.focusedOutlineColor = {255, 255, 0, 255};
    world.add(world.spawn({10.0f, 10.0f, 20.0f, 20.0f}), button);
    UIProgressBar bar;
    world.add(world.spawn({50.0f, 10.0f, 20.0f, 20.0f}), bar);
    engine::ecs::UITheme theme;
    theme.options().focusThickness = 4.0f;
    theme.options().gaugeOutline = 2.0f;
    sf::RenderTexture target;
    REQUIRE(target.resize({80, 40}));
    ResourceManager resources("does/not/exist");
    UIRenderSystem system(resources);
    system.setTheme(theme);

    target.clear(sf::Color::Black);
    system.render(target, world.registry);
    target.display();

    const sf::Image image = target.getTexture().copyToImage();
    REQUIRE(image.getPixel({7, 20}) == sf::Color::Yellow);
    REQUIRE(image.getPixel({49, 20}) == sf::Color::White);
}
