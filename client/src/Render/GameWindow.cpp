/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameWindow.cpp
*/

#include "Client/Render/GameWindow.hpp"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <array>
#include <cmath>
#include <filesystem>
#include <optional>
#include <vector>

#include "Client/Settings/Palette.hpp"

namespace client::render {

namespace {

using Scan = sf::Keyboard::Scan;

constexpr float WORLD_WIDTH = 1280.0F;
constexpr float WORLD_HEIGHT = 720.0F;
constexpr float SHIP_SCALE = 3.0F;
constexpr float BYDOS_SCALE = 2.5F;
constexpr unsigned SOUND_RATE = 22050;
constexpr std::uint8_t KIND_SHIP = 1;
constexpr std::uint8_t KIND_BYDOS = 2;
constexpr std::uint8_t KIND_PLAYER_MISSILE = 3;
constexpr std::uint8_t KIND_ENEMY_MISSILE = 4;
constexpr settings::ScanCode FIRST_HID_LETTER = 0x04;
constexpr settings::ScanCode LAST_HID_DIGIT = 0x27;

struct KeyPair {
    settings::ScanCode key;
    sf::Keyboard::Scancode scan;
};

constexpr std::array<KeyPair, 6> SPECIAL_KEYS = {{
    {settings::KEY_ESCAPE, Scan::Escape},
    {settings::KEY_SPACE, Scan::Space},
    {settings::KEY_RIGHT, Scan::Right},
    {settings::KEY_LEFT, Scan::Left},
    {settings::KEY_DOWN, Scan::Down},
    {settings::KEY_UP, Scan::Up},
}};

sf::Keyboard::Scancode toSfml(settings::ScanCode key)
{
    for (const KeyPair &pair : SPECIAL_KEYS) {
        if (pair.key == key) {
            return pair.scan;
        }
    }
    if (key >= FIRST_HID_LETTER && key <= LAST_HID_DIGIT) {
        return static_cast<sf::Keyboard::Scancode>(static_cast<int>(Scan::A) +
                                                   (key - FIRST_HID_LETTER));
    }
    return Scan::Unknown;
}

std::vector<sf::Vector2f> circlePoints()
{
    constexpr std::size_t COUNT = 16;
    constexpr float PI = 3.14159265F;
    std::vector<sf::Vector2f> points;

    for (std::size_t i = 0; i < COUNT; ++i) {
        const float angle =
            2.0F * PI * static_cast<float>(i) / static_cast<float>(COUNT);

        points.emplace_back(8.0F + 8.0F * std::cos(angle),
                            8.0F + 8.0F * std::sin(angle));
    }
    return points;
}

std::vector<sf::Vector2f> crossPoints()
{
    return {{5.0F, 0.0F},   {11.0F, 0.0F},  {11.0F, 5.0F},  {16.0F, 5.0F},
            {16.0F, 11.0F}, {11.0F, 11.0F}, {11.0F, 16.0F}, {5.0F, 16.0F},
            {5.0F, 11.0F},  {0.0F, 11.0F},  {0.0F, 5.0F},   {5.0F, 5.0F}};
}

std::vector<sf::Vector2f> markerPoints(unsigned slot)
{
    switch (slot % 4U) {
        case 0:
            return circlePoints();
        case 1:
            return {{0.0F, 0.0F}, {16.0F, 0.0F}, {8.0F, 14.0F}};
        case 2:
            return {{1.0F, 1.0F}, {15.0F, 1.0F}, {15.0F, 15.0F}, {1.0F, 15.0F}};
        default:
            return crossPoints();
    }
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
    unsigned seed = 12345U;

    for (std::size_t i = 0; i < count; ++i) {
        const float t = static_cast<float>(i) / static_cast<float>(count);
        const float hz = startHz + (endHz - startHz) * t;
        seed = seed * 1103515245U + 12345U;
        const float random =
            static_cast<float>((seed >> 16U) & 0x7FFFU) / 16384.0F - 1.0F;

        phase += 6.2831853F * hz / SOUND_RATE;
        const float wave = noise ? random : std::sin(phase);
        samples[i] = static_cast<std::int16_t>(wave * (1.0F - t) * 9000.0F);
    }
    return samples;
}

bool loadBuffer(sf::SoundBuffer &buffer,
                const std::vector<std::int16_t> &samples)
{
    return buffer.loadFromSamples(samples.data(), samples.size(), 1, SOUND_RATE,
                                  {sf::SoundChannel::Mono});
}

std::vector<sf::Color> playerColors(const Frame &frame)
{
    const auto theme =
        settings::makeTheme(frame.palette, frame.highContrast, false);

    return {toSfml(theme.resolve(engine::ecs::UIColorRole::PLAYER_1, {})),
            toSfml(theme.resolve(engine::ecs::UIColorRole::PLAYER_2, {})),
            toSfml(theme.resolve(engine::ecs::UIColorRole::PLAYER_3, {})),
            toSfml(theme.resolve(engine::ecs::UIColorRole::PLAYER_4, {}))};
}

} // namespace

/**
 * @brief The window library side of GameWindow: window, assets and drawing.
 */
class GameWindow::Impl {
public:
    Impl(const settings::ClientSettings &settings, const std::string &title,
         const std::string &assetRoot);

    [[nodiscard]] bool isOpen() const;
    void pollEvents();
    void close();
    void draw(const Frame &frame);
    void playShot();
    void playExplosion();

private:
    void fitView(sf::Vector2u size);
    void loadTextures(const std::filesystem::path &root);
    void loadFont(const std::filesystem::path &root);
    void loadSounds();
    void drawStars(const Frame &frame);
    void drawEntity(const world::RenderEntity &entity, sf::Color color,
                    bool local);
    [[nodiscard]] bool drawKnown(const world::RenderEntity &entity,
                                 sf::Vector2f at, sf::Color color, bool local);
    [[nodiscard]] bool drawSprite(const std::optional<sf::Texture> &texture,
                                  const sf::IntRect &rect, sf::Vector2f at,
                                  float scale);
    void drawBullet(sf::Vector2f at, bool fromPlayer);
    void drawPlaceholder(sf::Vector2f at, sf::Color color);
    void drawMarker(sf::Vector2f at, sf::Color color, bool local,
                    unsigned slot);
    void drawHud(const Frame &frame, const std::vector<sf::Color> &colors);
    void drawScores(const Frame &frame, const std::vector<sf::Color> &colors);
    void drawText(const std::string &text, sf::Vector2f at, unsigned size);

    sf::RenderWindow _window;
    sf::View _view{sf::FloatRect({0.0F, 0.0F}, {WORLD_WIDTH, WORLD_HEIGHT})};
    std::optional<sf::Font> _font;
    std::optional<sf::Texture> _ships;
    std::optional<sf::Texture> _bydos;
    sf::SoundBuffer _shotBuffer;
    sf::SoundBuffer _boomBuffer;
    std::optional<sf::Sound> _shot;
    std::optional<sf::Sound> _boom;
};

GameWindow::Impl::Impl(const settings::ClientSettings &settings,
                       const std::string &title, const std::string &assetRoot)
    : _window(sf::VideoMode(
                  {settings.resolution.width, settings.resolution.height}),
              title, sf::Style::Default,
              settings.fullscreen ? sf::State::Fullscreen : sf::State::Windowed)
{
    _window.setVerticalSyncEnabled(settings.vsync);
    fitView(_window.getSize());
    loadTextures(assetRoot);
    loadFont(assetRoot);
    loadSounds();
}

void GameWindow::Impl::fitView(sf::Vector2u size)
{
    const float windowRatio =
        static_cast<float>(size.x) / static_cast<float>(size.y);
    const float worldRatio = WORLD_WIDTH / WORLD_HEIGHT;
    sf::FloatRect viewport({0.0F, 0.0F}, {1.0F, 1.0F});

    if (windowRatio > worldRatio) {
        viewport.size.x = worldRatio / windowRatio;
        viewport.position.x = (1.0F - viewport.size.x) / 2.0F;
    } else {
        viewport.size.y = windowRatio / worldRatio;
        viewport.position.y = (1.0F - viewport.size.y) / 2.0F;
    }
    _view.setViewport(viewport);
}

void GameWindow::Impl::loadTextures(const std::filesystem::path &root)
{
    sf::Texture texture;

    if (texture.loadFromFile((root / "sprites/r-typesheet42.gif").string())) {
        _ships = texture;
    }
    if (texture.loadFromFile((root / "sprites/r-typesheet5.gif").string())) {
        _bydos = texture;
    }
}

void GameWindow::Impl::loadFont(const std::filesystem::path &root)
{
    const std::vector<std::filesystem::path> fonts = {
        root / "fonts/main.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/dejavu/DejaVuSans.ttf", "C:/Windows/Fonts/arial.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf"};

    for (const auto &path : fonts) {
        sf::Font candidate;

        if (std::filesystem::exists(path) &&
            candidate.openFromFile(path.string())) {
            _font = std::move(candidate);
            return;
        }
    }
}

void GameWindow::Impl::loadSounds()
{
    if (loadBuffer(_shotBuffer, tone(880.0F, 220.0F, 0.12F, false))) {
        _shot.emplace(_shotBuffer);
    }
    if (loadBuffer(_boomBuffer, tone(200.0F, 40.0F, 0.35F, true))) {
        _boom.emplace(_boomBuffer);
    }
}

void GameWindow::Impl::drawText(const std::string &text, sf::Vector2f at,
                                unsigned size)
{
    if (!_font) {
        return;
    }
    sf::Text label(*_font, text, size);

    label.setPosition(at);
    label.setFillColor(sf::Color::White);
    label.setOutlineColor(sf::Color::Black);
    label.setOutlineThickness(2.0F);
    _window.draw(label);
}

void GameWindow::Impl::drawStars(const Frame &frame)
{
    for (const scenes::Star &star : frame.stars.stars()) {
        sf::RectangleShape dot({star.size, star.size});

        dot.setPosition(
            {star.position.x * WORLD_WIDTH, star.position.y * WORLD_HEIGHT});
        dot.setFillColor({star.brightness, star.brightness, star.brightness});
        _window.draw(dot);
    }
}

bool GameWindow::Impl::drawSprite(const std::optional<sf::Texture> &texture,
                                  const sf::IntRect &rect, sf::Vector2f at,
                                  float scale)
{
    if (!texture) {
        return false;
    }
    sf::Sprite sprite(*texture);

    sprite.setTextureRect(rect);
    sprite.setPosition(at);
    sprite.setScale({scale, scale});
    _window.draw(sprite);
    return true;
}

void GameWindow::Impl::drawBullet(sf::Vector2f at, bool fromPlayer)
{
    sf::RectangleShape bullet({24.0F, 6.0F});

    bullet.setPosition(at);
    bullet.setFillColor(fromPlayer ? sf::Color(80, 220, 255)
                                   : sf::Color(255, 160, 40));
    _window.draw(bullet);
}

void GameWindow::Impl::drawPlaceholder(sf::Vector2f at, sf::Color color)
{
    sf::RectangleShape box({40.0F, 24.0F});

    box.setPosition(at);
    box.setFillColor(color);
    _window.draw(box);
}

void GameWindow::Impl::drawMarker(sf::Vector2f at, sf::Color color, bool local,
                                  unsigned slot)
{
    const std::vector<sf::Vector2f> points = markerPoints(slot);
    sf::ConvexShape marker(points.size());

    for (std::size_t i = 0; i < points.size(); ++i) {
        marker.setPoint(i, points[i]);
    }
    marker.setPosition({at.x + 40.0F, at.y - 16.0F});
    marker.setFillColor(color);
    marker.setOutlineColor(local ? sf::Color::White : sf::Color::Black);
    marker.setOutlineThickness(local ? 2.0F : 1.0F);
    _window.draw(marker);
}

bool GameWindow::Impl::drawKnown(const world::RenderEntity &entity,
                                 sf::Vector2f at, sf::Color color, bool local)
{
    switch (entity.kind) {
        case KIND_SHIP:
            drawMarker(at, color, local, entity.owner);
            return drawSprite(
                _ships, sf::IntRect({66, 17 * (entity.owner % 5)}, {33, 17}),
                at, SHIP_SCALE);
        case KIND_BYDOS:
            return drawSprite(_bydos, sf::IntRect({0, 0}, {33, 36}), at,
                              BYDOS_SCALE);
        case KIND_PLAYER_MISSILE:
        case KIND_ENEMY_MISSILE:
            drawBullet(at, entity.kind == KIND_PLAYER_MISSILE);
            return true;
        default:
            return false;
    }
}

void GameWindow::Impl::drawEntity(const world::RenderEntity &entity,
                                  sf::Color color, bool local)
{
    const sf::Vector2f at{entity.position.x, entity.position.y};

    if (!drawKnown(entity, at, color, local)) {
        drawPlaceholder(
            at, entity.kind == KIND_SHIP ? color : sf::Color(160, 160, 160));
    }
}

void GameWindow::Impl::drawScores(const Frame &frame,
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
        _window.draw(swatch);
        drawText("P" + std::to_string(slot + 1) + " " + info.name + "  " +
                     std::to_string(info.score),
                 {44.0F, y}, 22);
        y += 28.0F;
    }
}

void GameWindow::Impl::drawHud(const Frame &frame,
                               const std::vector<sf::Color> &colors)
{
    drawScores(frame, colors);
    drawText("Lives " + std::to_string(frame.world.lives()) + "   Level " +
                 std::to_string(frame.world.level()) + "   " +
                 std::to_string(frame.rttMs) + " ms",
             {WORLD_WIDTH - 360.0F, 12.0F}, 20);
    if (!frame.banner.empty()) {
        drawText(frame.banner,
                 {WORLD_WIDTH / 2.0F - 220.0F, WORLD_HEIGHT / 2.0F - 20.0F},
                 36);
    }
}

bool GameWindow::Impl::isOpen() const
{
    return _window.isOpen();
}

void GameWindow::Impl::pollEvents()
{
    while (const std::optional event = _window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            _window.close();
        } else if (const auto *resized = event->getIf<sf::Event::Resized>()) {
            fitView(resized->size);
        }
    }
}

void GameWindow::Impl::close()
{
    _window.close();
}

void GameWindow::Impl::draw(const Frame &frame)
{
    const std::vector<sf::Color> colors = playerColors(frame);

    _window.clear(sf::Color(5, 5, 20));
    _window.setView(_view);
    drawStars(frame);
    for (const world::RenderEntity &entity : frame.world.visible(frame.now)) {
        const bool local = entity.owner == frame.world.localSlot();

        drawEntity(entity, colors[entity.owner % colors.size()],
                   local && entity.kind == KIND_SHIP);
    }
    drawHud(frame, colors);
    _window.display();
}

void GameWindow::Impl::playShot()
{
    if (_shot) {
        _shot->play();
    }
}

void GameWindow::Impl::playExplosion()
{
    if (_boom) {
        _boom->play();
    }
}

GameWindow::GameWindow(const settings::ClientSettings &settings,
                       const std::string &title, const std::string &assetRoot)
    : _impl(std::make_unique<Impl>(settings, title, assetRoot))
{}

GameWindow::~GameWindow() = default;

bool GameWindow::isOpen() const
{
    return _impl->isOpen();
}

void GameWindow::pollEvents()
{
    _impl->pollEvents();
}

bool GameWindow::isKeyDown(settings::ScanCode key)
{
    const auto scan = toSfml(key);

    return scan != Scan::Unknown && sf::Keyboard::isKeyPressed(scan);
}

void GameWindow::draw(const Frame &frame)
{
    _impl->draw(frame);
}

void GameWindow::close()
{
    _impl->close();
}

void GameWindow::playShot()
{
    _impl->playShot();
}

void GameWindow::playExplosion()
{
    _impl->playExplosion();
}

} /* namespace client::render */
