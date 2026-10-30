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
#include "Engine/Ecs/Registry.hpp"

using engine::ecs::Entity;
using engine::ecs::Registry;
using engine::ecs::components::Rect;
using engine::ecs::components::UIBorders;
using engine::ecs::components::UIColor;
using engine::ecs::components::UIFillDirection;
using engine::ecs::components::UIPanel;
using engine::ecs::components::UIPanelMode;
using engine::ecs::components::UIProgressBar;
using engine::ecs::components::UIText;
using engine::ecs::components::UITextAlign;
using engine::ecs::components::UITransform;
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
