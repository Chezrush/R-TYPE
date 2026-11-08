/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** GameWindow.cpp
*/

#include "Client/Render/GameWindow.hpp"

#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <optional>
#include <vector>

#include "Client/Events/SettingsEvents.hpp"
#include "Client/Settings/Palette.hpp"
#include "Engine/Ecs/Components/UIComponents.hpp"
#include "Engine/Render/UIRenderSystem.hpp"
#include "Engine/Resources/ResourceManager.hpp"

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

constexpr settings::ScanCode KEY_ENTER = 0x28;
constexpr settings::ScanCode KEY_BACKSPACE = 0x2A;
constexpr settings::ScanCode KEY_TAB = 0x2B;
constexpr settings::ScanCode KEY_HOME = 0x4A;
constexpr settings::ScanCode KEY_DELETE = 0x4C;
constexpr settings::ScanCode KEY_END = 0x4D;
constexpr settings::ScanCode FIRST_HID_FUNCTION = 0x3A;
constexpr settings::ScanCode LAST_HID_FUNCTION = 0x45;

constexpr std::array<KeyPair, 13> SPECIAL_KEYS = {{
    {settings::KEY_ESCAPE, Scan::Escape},
    {settings::KEY_SPACE, Scan::Space},
    {settings::KEY_RIGHT, Scan::Right},
    {settings::KEY_LEFT, Scan::Left},
    {settings::KEY_DOWN, Scan::Down},
    {settings::KEY_UP, Scan::Up},
    {KEY_ENTER, Scan::Enter},
    {KEY_BACKSPACE, Scan::Backspace},
    {KEY_TAB, Scan::Tab},
    {settings::KEY_GRAVE, Scan::Grave},
    {KEY_HOME, Scan::Home},
    {KEY_DELETE, Scan::Delete},
    {KEY_END, Scan::End},
}};

sf::Keyboard::Scancode toSfml(settings::ScanCode key)
{
    for (const KeyPair &pair : SPECIAL_KEYS) {
        if (pair.key == key) {
            return pair.scan;
        }
    }
    if (key >= FIRST_HID_FUNCTION && key <= LAST_HID_FUNCTION) {
        return static_cast<sf::Keyboard::Scancode>(static_cast<int>(Scan::F1) +
                                                   (key - FIRST_HID_FUNCTION));
    }
    if (key >= FIRST_HID_LETTER && key <= LAST_HID_DIGIT) {
        return static_cast<sf::Keyboard::Scancode>(static_cast<int>(Scan::A) +
                                                   (key - FIRST_HID_LETTER));
    }
    return Scan::Unknown;
}

settings::ScanCode toHid(sf::Keyboard::Scancode scan)
{
    for (const KeyPair &pair : SPECIAL_KEYS) {
        if (pair.scan == scan) {
            return pair.key;
        }
    }
    const int code = static_cast<int>(scan);

    if (code >= static_cast<int>(Scan::F1) &&
        code <= static_cast<int>(Scan::F12)) {
        return static_cast<settings::ScanCode>(
            FIRST_HID_FUNCTION + (code - static_cast<int>(Scan::F1)));
    }
    if (code >= static_cast<int>(Scan::A) &&
        code <= static_cast<int>(Scan::Num0)) {
        return static_cast<settings::ScanCode>(
            FIRST_HID_LETTER + (code - static_cast<int>(Scan::A)));
    }
    return 0;
}

struct UiKeyPair {
    sf::Keyboard::Scancode scan;
    engine::ecs::UIKey key;
};

constexpr std::array<UiKeyPair, 12> UI_KEYS = {{
    {Scan::Tab, engine::ecs::UIKey::TAB},
    {Scan::Up, engine::ecs::UIKey::UP},
    {Scan::Down, engine::ecs::UIKey::DOWN},
    {Scan::Left, engine::ecs::UIKey::LEFT},
    {Scan::Right, engine::ecs::UIKey::RIGHT},
    {Scan::Enter, engine::ecs::UIKey::ENTER},
    {Scan::Space, engine::ecs::UIKey::SPACE},
    {Scan::Escape, engine::ecs::UIKey::ESCAPE},
    {Scan::Backspace, engine::ecs::UIKey::BACKSPACE},
    {Scan::Delete, engine::ecs::UIKey::DEL},
    {Scan::Home, engine::ecs::UIKey::HOME},
    {Scan::End, engine::ecs::UIKey::END},
}};

std::optional<engine::ecs::UIKey> toUiKey(const sf::Event::KeyPressed &key)
{
    for (const UiKeyPair &pair : UI_KEYS) {
        if (pair.scan != key.scancode) {
            continue;
        }
        if (pair.scan == Scan::Tab && key.shift) {
            return engine::ecs::UIKey::BACK_TAB;
        }
        return pair.key;
    }
    return std::nullopt;
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
    void drawUi();
    void playShot();
    void playExplosion();
    [[nodiscard]] engine::ecs::Registry &registry() noexcept;
    [[nodiscard]] engine::core::EventBus &bus() noexcept;
    [[nodiscard]] engine::ecs::UIInteractionSystem &interaction() noexcept;
    [[nodiscard]] engine::ecs::UILayoutSystem &layout() noexcept;
    [[nodiscard]] engine::ecs::UITheme &theme() noexcept;
    [[nodiscard]] const std::string &fontId() const noexcept;
    [[nodiscard]] engine::core::Vec2 size() const;
    [[nodiscard]] engine::core::Vec2 toScreen(engine::core::Vec2 world) const;

private:
    void fitView(sf::Vector2u size);
    void resize(sf::Vector2u size);
    void onEvent(const sf::Event &event);
    void onMouse(const sf::Event &event);
    void onButton(const sf::Event &event);
    void onKey(const sf::Event::KeyPressed &key);
    void onText(const sf::Event::TextEntered &text);
    void listenToSettings();
    void applyVideo(const events::VideoSettingsChanged &video);
    void applyVolume() const;
    void loadTextures(const std::filesystem::path &root);
    void loadFont(const std::filesystem::path &root);
    void loadSounds();
    void drawStars(const Frame &frame);
    void drawEntity(const world::RenderEntity &entity, sf::Color color);
    [[nodiscard]] bool drawKnown(const world::RenderEntity &entity,
                                 sf::Vector2f at);
    [[nodiscard]] bool drawSprite(const std::optional<sf::Texture> &texture,
                                  const sf::IntRect &rect, sf::Vector2f at,
                                  float scale);
    void drawBullet(sf::Vector2f at, bool fromPlayer);
    void drawPlaceholder(sf::Vector2f at, sf::Color color);
    void drawCues(const Frame &frame);
    void drawHud(const Frame &frame, const std::vector<sf::Color> &colors);
    void drawScores(const Frame &frame, const std::vector<sf::Color> &colors);
    void drawText(const std::string &text, sf::Vector2f at, unsigned size);

    sf::RenderWindow _window;
    std::string _title;
    sf::View _view{sf::FloatRect({0.0F, 0.0F}, {WORLD_WIDTH, WORLD_HEIGHT})};
    std::optional<sf::Font> _font;
    std::string _fontId;
    std::optional<sf::Texture> _ships;
    std::optional<sf::Texture> _bydos;
    sf::SoundBuffer _shotBuffer;
    sf::SoundBuffer _boomBuffer;
    std::optional<sf::Sound> _shot;
    std::optional<sf::Sound> _boom;
    int _masterVolume;
    int _sfxVolume;
    bool _muted;
    engine::ecs::Registry _registry;
    engine::core::EventBus _bus;
    engine::ecs::UIInteractionSystem _interaction;
    engine::ecs::UILayoutSystem _layout;
    engine::ecs::UITheme _theme;
    engine::resources::ResourceManager _resources;
    engine::render::UIRenderSystem _uiRender;
};

GameWindow::Impl::Impl(const settings::ClientSettings &settings,
                       const std::string &title, const std::string &assetRoot)
    : _window(sf::VideoMode(
                  {settings.resolution.width, settings.resolution.height}),
              title, sf::Style::Default,
              settings.fullscreen ? sf::State::Fullscreen
                                  : sf::State::Windowed),
      _title(title),
      _masterVolume(settings.masterVolume),
      _sfxVolume(settings.sfxVolume),
      _muted(settings.muted),
      _interaction(_bus),
      _theme(settings::makeTheme(settings.palette, settings.highContrast,
                                 settings.reducedMotion)),
      _resources(assetRoot),
      _uiRender(_resources)
{
    engine::ecs::registerUIComponents(_registry);
    _uiRender.setTheme(_theme);
    _window.setVerticalSyncEnabled(settings.vsync);
    resize(_window.getSize());
    loadTextures(assetRoot);
    loadFont(assetRoot);
    loadSounds();
    applyVolume();
    listenToSettings();
}

void GameWindow::Impl::resize(sf::Vector2u size)
{
    if (size.x == 0 || size.y == 0) {
        return;
    }
    fitView(size);
    _layout.setResolution(static_cast<float>(size.x),
                          static_cast<float>(size.y));
    _bus.publish(events::WindowResized{static_cast<float>(size.x),
                                       static_cast<float>(size.y)});
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
            _fontId = path == fonts.front() ? "fonts/main.ttf" : path.string();
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

bool GameWindow::Impl::drawKnown(const world::RenderEntity &entity,
                                 sf::Vector2f at)
{
    switch (entity.kind) {
        case KIND_SHIP:
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
                                  sf::Color color)
{
    const sf::Vector2f at{entity.position.x, entity.position.y};

    if (!drawKnown(entity, at)) {
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

void GameWindow::Impl::drawCues(const Frame &frame)
{
    const std::array<sf::Vector2f, 4> places = {
        {{16.0F, WORLD_HEIGHT / 2.0F},
         {WORLD_WIDTH - 200.0F, WORLD_HEIGHT / 2.0F},
         {WORLD_WIDTH / 2.0F - 80.0F, 48.0F},
         {WORLD_WIDTH / 2.0F - 80.0F, WORLD_HEIGHT - 60.0F}}};

    for (std::size_t side = 0; side < places.size(); ++side) {
        if (!frame.cues[side].empty()) {
            drawText(frame.cues[side], places[side], 24);
        }
    }
}

void GameWindow::Impl::drawHud(const Frame &frame,
                               const std::vector<sf::Color> &colors)
{
    drawScores(frame, colors);
    drawText(std::to_string(frame.rttMs) + " ms", {WORLD_WIDTH - 110.0F, 12.0F},
             20);
    drawCues(frame);
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
        onEvent(*event);
    }
}

void GameWindow::Impl::onEvent(const sf::Event &event)
{
    if (event.is<sf::Event::Closed>()) {
        _window.close();
    } else if (const auto *resized = event.getIf<sf::Event::Resized>()) {
        resize(resized->size);
    } else if (const auto *key = event.getIf<sf::Event::KeyPressed>()) {
        onKey(*key);
    } else if (const auto *text = event.getIf<sf::Event::TextEntered>()) {
        onText(*text);
    } else {
        onMouse(event);
    }
}

void GameWindow::Impl::onMouse(const sf::Event &event)
{
    if (const auto *moved = event.getIf<sf::Event::MouseMoved>()) {
        _interaction.onMouseMove(static_cast<float>(moved->position.x),
                                 static_cast<float>(moved->position.y));
    } else if (const auto *wheel =
                   event.getIf<sf::Event::MouseWheelScrolled>()) {
        _interaction.onMouseWheel(static_cast<float>(wheel->position.x),
                                  static_cast<float>(wheel->position.y),
                                  wheel->delta);
    } else {
        onButton(event);
    }
}

void GameWindow::Impl::onButton(const sf::Event &event)
{
    if (const auto *down = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (down->button == sf::Mouse::Button::Left) {
            _interaction.onMouseDown(static_cast<float>(down->position.x),
                                     static_cast<float>(down->position.y));
        }
    } else if (const auto *up = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (up->button == sf::Mouse::Button::Left) {
            _interaction.onMouseUp(static_cast<float>(up->position.x),
                                   static_cast<float>(up->position.y));
        }
    }
}

void GameWindow::Impl::onKey(const sf::Event::KeyPressed &key)
{
    const settings::ScanCode hid = toHid(key.scancode);

    if (hid != 0) {
        _bus.publish(events::RawKeyPressed{hid});
    }
    if (const auto uiKey = toUiKey(key)) {
        _interaction.onKeyPressed(*uiKey);
    }
}

void GameWindow::Impl::onText(const sf::Event::TextEntered &text)
{
    constexpr char32_t FIRST_PRINTABLE = 32;
    constexpr char32_t DELETE_CHARACTER = 127;

    if (text.unicode >= FIRST_PRINTABLE && text.unicode != DELETE_CHARACTER) {
        _interaction.onTextEntered(text.unicode);
    }
}

void GameWindow::Impl::listenToSettings()
{
    _bus.subscribe<events::VideoSettingsChanged>(
        [this](const events::VideoSettingsChanged &video) {
            applyVideo(video);
        });
    _bus.subscribe<events::VolumeChanged>(
        [this](const events::VolumeChanged &volume) {
            if (volume.bus == events::VolumeBus::MASTER) {
                _masterVolume = volume.value;
            } else if (volume.bus == events::VolumeBus::SFX) {
                _sfxVolume = volume.value;
            }
            applyVolume();
        });
    _bus.subscribe<events::MuteChanged>(
        [this](const events::MuteChanged &mute) {
            _muted = mute.muted;
            applyVolume();
        });
}

void GameWindow::Impl::applyVideo(const events::VideoSettingsChanged &video)
{
    _window.create(
        sf::VideoMode({video.resolution.width, video.resolution.height}),
        _title, sf::Style::Default,
        video.fullscreen ? sf::State::Fullscreen : sf::State::Windowed);
    _window.setVerticalSyncEnabled(video.vsync);
    resize(_window.getSize());
}

void GameWindow::Impl::applyVolume() const
{
    sf::Listener::setGlobalVolume(_muted ? 0.0F
                                         : static_cast<float>(_masterVolume));
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
        drawEntity(entity, colors[entity.owner % colors.size()]);
    }
    drawHud(frame, colors);
    _uiRender.render(_window, _registry);
    _window.display();
}

void GameWindow::Impl::drawUi()
{
    _window.clear(sf::Color(5, 5, 20));
    _uiRender.render(_window, _registry);
    _window.display();
}

engine::ecs::Registry &GameWindow::Impl::registry() noexcept
{
    return _registry;
}

engine::core::EventBus &GameWindow::Impl::bus() noexcept
{
    return _bus;
}

engine::ecs::UIInteractionSystem &GameWindow::Impl::interaction() noexcept
{
    return _interaction;
}

engine::ecs::UILayoutSystem &GameWindow::Impl::layout() noexcept
{
    return _layout;
}

engine::ecs::UITheme &GameWindow::Impl::theme() noexcept
{
    return _theme;
}

const std::string &GameWindow::Impl::fontId() const noexcept
{
    return _fontId;
}

engine::core::Vec2 GameWindow::Impl::size() const
{
    const sf::Vector2u pixels = _window.getSize();

    return {static_cast<float>(pixels.x), static_cast<float>(pixels.y)};
}

engine::core::Vec2 GameWindow::Impl::toScreen(engine::core::Vec2 world) const
{
    const sf::FloatRect viewport = _view.getViewport();
    const engine::core::Vec2 pixels = size();

    return {(viewport.position.x + world.x / WORLD_WIDTH * viewport.size.x) *
                pixels.x,
            (viewport.position.y + world.y / WORLD_HEIGHT * viewport.size.y) *
                pixels.y};
}

void GameWindow::Impl::playShot()
{
    if (_shot) {
        _shot->setVolume(static_cast<float>(_sfxVolume));
        _shot->play();
    }
}

void GameWindow::Impl::playExplosion()
{
    if (_boom) {
        _boom->setVolume(static_cast<float>(_sfxVolume));
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

void GameWindow::drawUi()
{
    _impl->drawUi();
}

engine::ecs::Registry &GameWindow::registry() noexcept
{
    return _impl->registry();
}

engine::core::EventBus &GameWindow::bus() noexcept
{
    return _impl->bus();
}

engine::ecs::UIInteractionSystem &GameWindow::interaction() noexcept
{
    return _impl->interaction();
}

engine::ecs::UILayoutSystem &GameWindow::layout() noexcept
{
    return _impl->layout();
}

engine::ecs::UITheme &GameWindow::theme() noexcept
{
    return _impl->theme();
}

const std::string &GameWindow::fontId() const noexcept
{
    return _impl->fontId();
}

engine::core::Vec2 GameWindow::size() const
{
    return _impl->size();
}

engine::core::Vec2 GameWindow::toScreen(engine::core::Vec2 world) const
{
    return _impl->toScreen(world);
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
