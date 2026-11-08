/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameWindow.cpp
*/

#include "Client/Render/GameWindow.hpp"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <cmath>
#include <filesystem>
#include <optional>
#include <vector>

#include "Client/Settings/Palette.hpp"

namespace client::render {

namespace {

constexpr float WORLD_WIDTH = 1280.0F;
constexpr float WORLD_HEIGHT = 720.0F;
constexpr float SHIP_SCALE = 3.0F;
constexpr float BYDOS_SCALE = 2.5F;
constexpr unsigned SOUND_RATE = 22050;

sf::Keyboard::Scancode toSfml(settings::ScanCode key)
{
    using Scan = sf::Keyboard::Scan;

    switch (key) {
        case settings::KEY_ESCAPE:
            return Scan::Escape;
        case settings::KEY_SPACE:
            return Scan::Space;
        case settings::KEY_RIGHT:
            return Scan::Right;
        case settings::KEY_LEFT:
            return Scan::Left;
        case settings::KEY_DOWN:
            return Scan::Down;
        case settings::KEY_UP:
            return Scan::Up;
        default:
            break;
    }
    if (key >= 0x04 && key <= 0x27) {
        return static_cast<sf::Keyboard::Scancode>(static_cast<int>(Scan::A) +
                                                   (key - 0x04));
    }
    return Scan::Unknown;
}

sf::Color toSfml(engine::ecs::UIColor color)
{
    return {color.r, color.g, color.b, color.a};
}

std::vector<std::int16_t> tone(float startHz, float endHz, float seconds,
                               bool noise)
{
    const auto count = static_cast<std::size_t>(seconds * SOUND_RATE);
    std::vector<std::int16_t> samples(count);
    float phase = 0.0F;
    unsigned seed = 12345;

    for (std::size_t i = 0; i < count; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(count);
        const float hz = startHz + (endHz - startHz) * t;
        seed = seed * 1103515245U + 12345U;
        const float random =
            static_cast<float>((seed >> 16) & 0x7FFF) / 16384.0F - 1.0F;

        phase += 6.2831853F * hz / SOUND_RATE;
        const float wave = noise ? random : std::sin(phase);
        samples[i] = static_cast<std::int16_t>(wave * (1.0F - t) * 9000.0F);
    }
    return samples;
}

} // namespace

struct GameWindow::Impl {
    sf::RenderWindow window;
    sf::View view{sf::FloatRect({0.0F, 0.0F}, {WORLD_WIDTH, WORLD_HEIGHT})};
    std::optional<sf::Font> font;
    std::optional<sf::Texture> ships;
    std::optional<sf::Texture> bydos;
    sf::SoundBuffer shotBuffer;
    sf::SoundBuffer boomBuffer;
    std::optional<sf::Sound> shot;
    std::optional<sf::Sound> boom;

    Impl(const settings::ClientSettings &settings, const std::string &title)
        : window(sf::VideoMode(
                     {settings.resolution.width, settings.resolution.height}),
                 title, sf::Style::Default,
                 settings.fullscreen ? sf::State::Fullscreen
                                     : sf::State::Windowed)
    {
        window.setVerticalSyncEnabled(settings.vsync);
        fitView(window.getSize());
    }

    void fitView(sf::Vector2u size)
    {
        const float window_ratio =
            static_cast<float>(size.x) / static_cast<float>(size.y);
        const float world_ratio = WORLD_WIDTH / WORLD_HEIGHT;
        sf::FloatRect viewport({0.0F, 0.0F}, {1.0F, 1.0F});

        if (window_ratio > world_ratio) {
            viewport.size.x = world_ratio / window_ratio;
            viewport.position.x = (1.0F - viewport.size.x) / 2.0F;
        } else {
            viewport.size.y = window_ratio / world_ratio;
            viewport.position.y = (1.0F - viewport.size.y) / 2.0F;
        }
        view.setViewport(viewport);
    }

    void loadAssets(const std::string &root);
    void drawStars(const Frame &frame);
    void drawEntity(const world::RenderEntity &entity, sf::Color color,
                    bool local);
    void drawHud(const Frame &frame, const std::vector<sf::Color> &colors);
    void drawText(const std::string &text, sf::Vector2f at, unsigned size,
                  sf::Color color);
};

void GameWindow::Impl::loadAssets(const std::string &root)
{
    const std::filesystem::path base(root);
    sf::Texture texture;

    if (texture.loadFromFile((base / "sprites/r-typesheet42.gif").string())) {
        ships = texture;
    }
    if (texture.loadFromFile((base / "sprites/r-typesheet5.gif").string())) {
        bydos = texture;
    }
    const std::vector<std::filesystem::path> fonts = {
        base / "fonts/main.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/dejavu/DejaVuSans.ttf", "C:/Windows/Fonts/arial.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf"};

    for (const auto &path : fonts) {
        sf::Font candidate;

        if (std::filesystem::exists(path) &&
            candidate.openFromFile(path.string())) {
            font = std::move(candidate);
            break;
        }
    }
    const auto shotSamples = tone(880.0F, 220.0F, 0.12F, false);
    const auto boomSamples = tone(200.0F, 40.0F, 0.35F, true);

    if (shotBuffer.loadFromSamples(shotSamples.data(), shotSamples.size(), 1,
                                   SOUND_RATE, {sf::SoundChannel::Mono})) {
        shot.emplace(shotBuffer);
    }
    if (boomBuffer.loadFromSamples(boomSamples.data(), boomSamples.size(), 1,
                                   SOUND_RATE, {sf::SoundChannel::Mono})) {
        boom.emplace(boomBuffer);
    }
}

void GameWindow::Impl::drawText(const std::string &text, sf::Vector2f at,
                                unsigned size, sf::Color color)
{
    if (!font) {
        return;
    }
    sf::Text label(*font, text, size);

    label.setPosition(at);
    label.setFillColor(color);
    label.setOutlineColor(sf::Color::Black);
    label.setOutlineThickness(2.0F);
    window.draw(label);
}

void GameWindow::Impl::drawStars(const Frame &frame)
{
    for (const scenes::Star &star : frame.stars.stars()) {
        sf::RectangleShape dot({star.size, star.size});

        dot.setPosition(
            {star.position.x * WORLD_WIDTH, star.position.y * WORLD_HEIGHT});
        dot.setFillColor({star.brightness, star.brightness, star.brightness});
        window.draw(dot);
    }
}

void GameWindow::Impl::drawEntity(const world::RenderEntity &entity,
                                  sf::Color color, bool local)
{
    const sf::Vector2f at{entity.position.x, entity.position.y};

    if (entity.kind == 1 && ships) {
        sf::Sprite sprite(*ships);
        const int row = entity.owner % 5;

        sprite.setTextureRect(sf::IntRect({66, 17 * row}, {33, 17}));
        sprite.setPosition(at);
        sprite.setScale({SHIP_SCALE, SHIP_SCALE});
        window.draw(sprite);
    } else if (entity.kind == 2 && bydos) {
        sf::Sprite sprite(*bydos);

        sprite.setTextureRect(sf::IntRect({0, 0}, {33, 36}));
        sprite.setPosition(at);
        sprite.setScale({BYDOS_SCALE, BYDOS_SCALE});
        window.draw(sprite);
    } else if (entity.kind == 3 || entity.kind == 4) {
        sf::RectangleShape bullet({24.0F, 6.0F});

        bullet.setPosition(at);
        bullet.setFillColor(entity.kind == 3 ? sf::Color(80, 220, 255)
                                             : sf::Color(255, 160, 40));
        window.draw(bullet);
    } else {
        sf::RectangleShape box({40.0F, 24.0F});

        box.setPosition(at);
        box.setFillColor(entity.kind == 1 ? color : sf::Color(160, 160, 160));
        window.draw(box);
    }
    if (entity.kind == 1) {
        sf::ConvexShape marker(3);

        marker.setPoint(0, {0.0F, 0.0F});
        marker.setPoint(1, {16.0F, 0.0F});
        marker.setPoint(2, {8.0F, 10.0F});
        marker.setPosition({at.x + 40.0F, at.y - 16.0F});
        marker.setFillColor(color);
        marker.setOutlineColor(local ? sf::Color::White : sf::Color::Black);
        marker.setOutlineThickness(local ? 2.0F : 1.0F);
        window.draw(marker);
    }
}

void GameWindow::Impl::drawHud(const Frame &frame,
                               const std::vector<sf::Color> &colors)
{
    float y = 12.0F;

    for (std::size_t slot = 0; slot < frame.world.players().size(); ++slot) {
        const world::PlayerInfo &info = frame.world.players()[slot];

        if (!info.present) {
            continue;
        }
        sf::RectangleShape swatch({18.0F, 18.0F});

        swatch.setPosition({16.0F, y + 4.0F});
        swatch.setFillColor(colors[slot % colors.size()]);
        window.draw(swatch);
        drawText("P" + std::to_string(slot + 1) + " " + info.name + "  " +
                     std::to_string(info.score),
                 {44.0F, y}, 22, sf::Color::White);
        y += 28.0F;
    }
    drawText("Level " + std::to_string(frame.world.level()) + "   " +
                 std::to_string(frame.rttMs) + " ms",
             {WORLD_WIDTH - 220.0F, 12.0F}, 20, sf::Color::White);
    if (!frame.banner.empty()) {
        drawText(frame.banner,
                 {WORLD_WIDTH / 2.0F - 220.0F, WORLD_HEIGHT / 2.0F - 20.0F}, 36,
                 sf::Color::White);
    }
}

GameWindow::GameWindow(const settings::ClientSettings &settings,
                       const std::string &title, const std::string &assetRoot)
    : _impl(std::make_unique<Impl>(settings, title))
{
    _impl->loadAssets(assetRoot);
}

GameWindow::~GameWindow() = default;

bool GameWindow::isOpen() const
{
    return _impl->window.isOpen();
}

void GameWindow::pollEvents()
{
    while (const std::optional event = _impl->window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            _impl->window.close();
        } else if (const auto *resized = event->getIf<sf::Event::Resized>()) {
            _impl->fitView(resized->size);
        }
    }
}

bool GameWindow::isKeyDown(settings::ScanCode key) const
{
    const auto scan = toSfml(key);

    return scan != sf::Keyboard::Scancode::Unknown &&
           sf::Keyboard::isKeyPressed(scan);
}

void GameWindow::draw(const Frame &frame)
{
    const auto theme =
        settings::makeTheme(frame.palette, frame.highContrast, false);
    const std::vector<sf::Color> colors = {
        toSfml(theme.resolve(engine::ecs::UIColorRole::PLAYER_1, {})),
        toSfml(theme.resolve(engine::ecs::UIColorRole::PLAYER_2, {})),
        toSfml(theme.resolve(engine::ecs::UIColorRole::PLAYER_3, {})),
        toSfml(theme.resolve(engine::ecs::UIColorRole::PLAYER_4, {}))};

    _impl->window.clear(sf::Color(5, 5, 20));
    _impl->window.setView(_impl->view);
    _impl->drawStars(frame);
    for (const world::RenderEntity &entity : frame.world.visible(frame.now)) {
        const bool local = entity.owner == frame.world.localSlot();

        _impl->drawEntity(entity, colors[entity.owner % colors.size()],
                          local && entity.kind == 1);
    }
    _impl->drawHud(frame, colors);
    _impl->window.display();
}

void GameWindow::close()
{
    _impl->window.close();
}

void GameWindow::playShot()
{
    if (_impl->shot) {
        _impl->shot->play();
    }
}

void GameWindow::playExplosion()
{
    if (_impl->boom) {
        _impl->boom->play();
    }
}

} /* namespace client::render */
