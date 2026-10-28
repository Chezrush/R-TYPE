/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** HudDemo.cpp
*/

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <optional>
#include <random>
#include <string>
#include <vector>

#include "Client/Events/GameEvents.hpp"
#include "Client/Hud/HudController.hpp"
#include "Client/Hud/HudView.hpp"
#include "Engine/Core/EventBus.hpp"
#include "Engine/Render/UIRenderSystem.hpp"
#include "Engine/Resources/ResourceManager.hpp"

namespace {

namespace events = client::events;

constexpr unsigned int WINDOW_WIDTH = 1280;
constexpr unsigned int WINDOW_HEIGHT = 720;
constexpr float SCROLL_SPEED = 90.0f;
constexpr float SHIP_SPEED = 320.0f;
constexpr float SHIP_SCALE = 2.0f;
constexpr sf::Vector2i SHIP_FRAME{33, 17};
constexpr float RESPAWN_DELAY = 2.0f;
constexpr float PASSIVE_SCORE_PERIOD = 0.5f;
constexpr std::uint32_t PASSIVE_SCORE = 50;
constexpr std::uint32_t BEAM_SHOT_SCORE = 500;
constexpr std::size_t STAR_COUNT = 220;
constexpr std::uint8_t LOCAL_PLAYER = 0;
constexpr std::uint8_t TOGGLED_PLAYER = 2;
constexpr std::array<sf::Vector2f, client::hud::MAX_PLAYERS> START_POSITIONS{{
    {200.0f, 300.0f},
    {160.0f, 160.0f},
    {160.0f, 440.0f},
    {260.0f, 540.0f},
}};
constexpr const char *SHIP_SHEET = "sprites/r-typesheet42.gif";

std::string findFont()
{
    const std::array<std::filesystem::path, 4> candidates{
        std::filesystem::path(RTYPE_ASSETS_DIR) / "fonts" / "hud.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf",
        "/usr/share/fonts/TTF/DejaVuSansMono-Bold.ttf",
        "C:/Windows/Fonts/consolab.ttf"};

    for (const auto &candidate : candidates) {
        if (std::filesystem::exists(candidate))
            return candidate.string();
    }
    return {};
}

std::uint16_t remoteNetId(std::uint8_t player) noexcept
{
    return static_cast<std::uint16_t>(1000 + player);
}

/**
 * @brief Fake game session: plays the server's part through the EventBus.
 */
class HudDemo {
public:
    explicit HudDemo(const std::string &fontId);

    void run();

private:
    void startSession();
    void handleEvent(const sf::Event &event);
    void handleKey(sf::Keyboard::Key key);
    void destroyLocalShip();
    void spawnLocalShip();
    void joinPlayer(std::uint8_t player);
    void toggleTeammate();
    void addScore(std::uint32_t points);
    void simulate(float dt);
    void respawn(float dt);
    void moveShips(float dt);
    void scrollWorld(float dt);
    void draw();
    void drawStars();
    void drawShips();
    void placeMarkers();
    [[nodiscard]] bool isShipVisible(std::size_t player) const;
    [[nodiscard]] sf::Vector2f toWorld(sf::Vector2f screen) const;

    sf::RenderWindow _window;
    sf::View _worldView;
    engine::core::EventBus _bus;
    client::hud::HudController _controller;
    client::hud::HudView _hud;
    engine::resources::ResourceManager _resources;
    engine::render::UIRenderSystem _uiRenderer;
    std::vector<sf::Vector2f> _stars;
    std::array<sf::Vector2f, client::hud::MAX_PLAYERS> _ships = START_POSITIONS;
    float _time = 0.0f;
    float _scoreTimer = 0.0f;
    float _respawnTimer = 0.0f;
    std::uint16_t _nextNetId = 1;
    std::uint16_t _localNetId = 0;
    std::uint32_t _score = 0;
    std::uint8_t _level = 1;
    bool _teammateOnline = false;
    bool _shooting = false;
};

HudDemo::HudDemo(const std::string &fontId)
    : _window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "R-Type HUD demo"),
      _worldView(
          sf::FloatRect({0.0f, 0.0f}, {static_cast<float>(WINDOW_WIDTH),
                                       static_cast<float>(WINDOW_HEIGHT)})),
      _controller(_bus),
      _hud(fontId, SHIP_SHEET),
      _resources(RTYPE_ASSETS_DIR),
      _uiRenderer(_resources)
{
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> x(0.0f, WINDOW_WIDTH);
    std::uniform_real_distribution<float> y(0.0f, WINDOW_HEIGHT - 48.0f);

    _window.setFramerateLimit(60);
    for (std::size_t i = 0; i < STAR_COUNT; ++i)
        _stars.emplace_back(x(rng), y(rng));
}

void HudDemo::run()
{
    sf::Clock clock;

    startSession();
    while (_window.isOpen()) {
        while (const std::optional event = _window.pollEvent())
            handleEvent(*event);
        const float dt = std::min(clock.restart().asSeconds(), 0.1f);
        scrollWorld(dt);
        moveShips(dt);
        simulate(dt);
        draw();
    }
}

void HudDemo::startSession()
{
    _bus.publish(events::WelcomeReceived{LOCAL_PLAYER, 4});
    _bus.publish(events::PlayerJoined{LOCAL_PLAYER, "You"});
    spawnLocalShip();
    for (const std::uint8_t player : {1, 3})
        joinPlayer(player);
    _bus.publish(events::GameStateChanged{events::GamePhase::PLAYING, _level});
    _bus.publish(events::ScoreChanged{LOCAL_PLAYER, _score});
}

void HudDemo::spawnLocalShip()
{
    _localNetId = _nextNetId++;
    _ships.at(LOCAL_PLAYER) = START_POSITIONS.at(LOCAL_PLAYER);
    _bus.publish(events::EntitySpawned{
        _localNetId, events::EntityKind::PLAYER_SHIP, LOCAL_PLAYER});
}

void HudDemo::joinPlayer(std::uint8_t player)
{
    _bus.publish(events::PlayerJoined{player, "Bot " + std::to_string(player)});
    _bus.publish(events::EntitySpawned{
        remoteNetId(player), events::EntityKind::PLAYER_SHIP, player});
}

void HudDemo::handleEvent(const sf::Event &event)
{
    if (event.is<sf::Event::Closed>())
        _window.close();
    if (const auto *key = event.getIf<sf::Event::KeyPressed>())
        handleKey(key->code);
    if (const auto *resized = event.getIf<sf::Event::Resized>()) {
        const sf::Vector2f size(resized->size);
        _hud.resize(size.x, size.y);
        _worldView.setSize(size);
        _worldView.setCenter({_worldView.getCenter().x, size.y / 2.0f});
    }
}

void HudDemo::handleKey(sf::Keyboard::Key key)
{
    switch (key) {
        case sf::Keyboard::Key::Escape:
            _window.close();
            break;
        case sf::Keyboard::Key::K:
            destroyLocalShip();
            break;
        case sf::Keyboard::Key::J:
            toggleTeammate();
            break;
        case sf::Keyboard::Key::N:
            _bus.publish(
                events::GameStateChanged{events::GamePhase::PLAYING, ++_level});
            break;
        default:
            break;
    }
}

void HudDemo::destroyLocalShip()
{
    if (!_controller.state().localAlive)
        return;
    _bus.publish(events::EntityDestroyed{
        _localNetId, events::DestroyReason::KILLED, events::NO_PLAYER});
    _respawnTimer = RESPAWN_DELAY;
}

void HudDemo::toggleTeammate()
{
    _teammateOnline = !_teammateOnline;
    if (_teammateOnline) {
        joinPlayer(TOGGLED_PLAYER);
        return;
    }
    _bus.publish(events::EntityDestroyed{remoteNetId(TOGGLED_PLAYER),
                                         events::DestroyReason::GONE,
                                         events::NO_PLAYER});
    _bus.publish(events::PlayerLeft{TOGGLED_PLAYER, 0});
}

void HudDemo::addScore(std::uint32_t points)
{
    _score += points;
    _bus.publish(events::ScoreChanged{LOCAL_PLAYER, _score});
}

void HudDemo::simulate(float dt)
{
    const bool shoot = _window.hasFocus() &&
                       sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);

    if (_shooting && !shoot && _controller.state().beamCharge >= 1.0f)
        addScore(BEAM_SHOT_SCORE);
    _shooting = shoot;
    _controller.update(dt, shoot);
    _scoreTimer += dt;
    if (_scoreTimer >= PASSIVE_SCORE_PERIOD) {
        _scoreTimer -= PASSIVE_SCORE_PERIOD;
        if (_controller.state().localAlive)
            addScore(PASSIVE_SCORE);
    }
    respawn(dt);
}

void HudDemo::respawn(float dt)
{
    if (_respawnTimer <= 0.0f)
        return;
    _respawnTimer -= dt;
    if (_respawnTimer > 0.0f)
        return;
    spawnLocalShip();
}

void HudDemo::moveShips(float dt)
{
    using sf::Keyboard::isKeyPressed;
    using Key = sf::Keyboard::Key;
    const sf::Vector2f input{
        static_cast<float>(isKeyPressed(Key::Right) - isKeyPressed(Key::Left)),
        static_cast<float>(isKeyPressed(Key::Down) - isKeyPressed(Key::Up))};
    const sf::Vector2f limit = _worldView.getSize() -
                               sf::Vector2f(SHIP_FRAME) * SHIP_SCALE -
                               sf::Vector2f{0.0f, 48.0f};
    sf::Vector2f &local = _ships.at(LOCAL_PLAYER);

    _time += dt;
    if (_window.hasFocus())
        local += input * SHIP_SPEED * dt;
    local = {std::clamp(local.x, 0.0f, limit.x),
             std::clamp(local.y, 0.0f, limit.y)};
    for (std::size_t i = 1; i < _ships.size(); ++i)
        _ships.at(i).y = START_POSITIONS.at(i).y +
                         40.0f * std::sin(_time * 1.3f + static_cast<float>(i));
}

void HudDemo::scrollWorld(float dt)
{
    const float left = _worldView.getCenter().x - _worldView.getSize().x / 2.0f;

    _worldView.move({SCROLL_SPEED * dt, 0.0f});
    for (sf::Vector2f &star : _stars) {
        if (star.x < left)
            star.x += _worldView.getSize().x;
    }
}

void HudDemo::draw()
{
    _window.clear(sf::Color(0, 0, 10));
    _window.setView(_worldView);
    drawStars();
    drawShips();
    placeMarkers();
    _hud.sync(_controller.state());
    _uiRenderer.render(_window, _hud.transforms(), _hud.panels(), _hud.bars(),
                       _hud.texts());
    _window.display();
}

void HudDemo::drawStars()
{
    sf::RectangleShape dot({2.0f, 2.0f});

    for (std::size_t i = 0; i < _stars.size(); ++i) {
        const std::uint8_t light = i % 3 == 0 ? 255 : 140;
        dot.setFillColor(sf::Color(light, light, light));
        dot.setPosition(_stars.at(i));
        _window.draw(dot);
    }
}

void HudDemo::drawShips()
{
    const sf::Texture *sheet = _resources.texture(SHIP_SHEET);

    if (sheet == nullptr)
        return;
    for (std::size_t i = 0; i < _ships.size(); ++i) {
        if (!isShipVisible(i))
            continue;
        const int row = static_cast<int>(i) * SHIP_FRAME.y;
        sf::Sprite ship(*sheet,
                        sf::IntRect({2 * SHIP_FRAME.x, row}, SHIP_FRAME));
        ship.setScale({SHIP_SCALE, SHIP_SCALE});
        ship.setPosition(toWorld(_ships.at(i)));
        _window.draw(ship);
    }
}

void HudDemo::placeMarkers()
{
    const float halfWidth = static_cast<float>(SHIP_FRAME.x) * SHIP_SCALE / 2;

    for (std::size_t i = 0; i < _ships.size(); ++i) {
        if (!isShipVisible(i)) {
            _hud.setMarker(i, std::nullopt);
            continue;
        }
        const sf::Vector2i pixel = _window.mapCoordsToPixel(
            toWorld(_ships.at(i)) + sf::Vector2f{halfWidth, 0.0f}, _worldView);
        _hud.setMarker(
            i, engine::ecs::components::Vec2{static_cast<float>(pixel.x),
                                             static_cast<float>(pixel.y)});
    }
}

bool HudDemo::isShipVisible(std::size_t player) const
{
    if (player == LOCAL_PLAYER)
        return _controller.state().localAlive;
    if (player == TOGGLED_PLAYER)
        return _teammateOnline;
    return true;
}

sf::Vector2f HudDemo::toWorld(sf::Vector2f screen) const
{
    return _worldView.getCenter() - _worldView.getSize() / 2.0f + screen;
}

} // namespace

int main()
{
    const std::string font = findFont();

    if (font.empty())
        std::cerr << "No font found: put one in assets/fonts/hud.ttf\n";
    std::cout << "Arrows: move  Space: charge beam (release when full: +500)\n"
              << "K: lose a life  J: P3 joins/leaves  N: next stage  "
              << "Esc: quit\n";
    HudDemo demo(font);
    demo.run();
    return 0;
}
