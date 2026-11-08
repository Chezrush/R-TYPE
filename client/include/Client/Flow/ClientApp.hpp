/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ClientApp.hpp
*/

#ifndef CLIENTAPP_HPP_
#define CLIENTAPP_HPP_

#include <array>
#include <memory>
#include <optional>
#include <string>

#include "Client/Events/ConnectionEvents.hpp"
#include "Client/Flow/GameScene.hpp"
#include "Client/Flow/IGameDevice.hpp"
#include "Client/Hud/StatsOverlay.hpp"
#include "Client/Net/ClientSession.hpp"
#include "Client/Render/Frame.hpp"
#include "Client/Scenes/ConnectForm.hpp"
#include "Client/Scenes/SceneRoute.hpp"
#include "Client/Settings/SettingsStore.hpp"
#include "Engine/Core/EventBus.hpp"
#include "Engine/Core/SceneManager.hpp"
#include "Engine/Ecs/Registry.hpp"
#include "Engine/Ecs/Systems/UIInteractionSystem.hpp"
#include "Engine/Ecs/Systems/UILayoutSystem.hpp"
#include "Engine/Ecs/UIScene.hpp"

namespace client::flow {

/** @brief What the application borrows from the window and the main. */
struct AppServices {
    IGameDevice &device;
    engine::ecs::Registry &registry;
    engine::core::EventBus &bus;
    engine::ecs::UIInteractionSystem &interaction;
    engine::ecs::UILayoutSystem &layout;
    engine::ecs::UITheme &theme;
    settings::SettingsStore &store;
    std::string fontId;
    scenes::ConnectForm prefill;
};

/**
 * @brief The screens of the client and the way from one to the next.
 *
 * Main menu -> connection form -> game, with the settings, controls and how
 * to play screens above the menu or the game. Joining a server creates the
 * ClientSession; the handshake result goes back to the form as
 * ConnectionFailed or WelcomeReceived. Leaving the game (menu, game over,
 * quit, lost connection) ends the session; a lost connection comes back to
 * the main menu with a notice. It knows nothing of the window library: the
 * main loop gives it the frame time and draws what it says.
 */
class ClientApp {
public:
    explicit ClientApp(AppServices services);
    ~ClientApp();

    ClientApp(const ClientApp &) = delete;
    ClientApp &operator=(const ClientApp &) = delete;
    ClientApp(ClientApp &&) = delete;
    ClientApp &operator=(ClientApp &&) = delete;

    /** @brief Runs one frame: the UI input, the scenes and the navigation. */
    void frame(float dt);

    /** @brief Whether the player asked to quit. */
    [[nodiscard]] bool quitRequested() const noexcept;

    /** @brief Leaves every scene, which says goodbye to the server. */
    void shutdown();

    /** @brief The world to draw under the UI, only while a game runs. */
    [[nodiscard]] std::optional<render::Frame> gameFrame() const;

    [[nodiscard]] const engine::core::IScene &currentScene() const;
    [[nodiscard]] const std::string &notice() const noexcept;
    [[nodiscard]] bool isConnecting() const noexcept;
    [[nodiscard]] const hud::StatsOverlay &stats() const noexcept;

private:
    void registerRoutes();
    void listen();
    void onConnect(const events::ConnectRequested &request);
    void onCancel();
    void onLost();
    void watchHandshake();
    void failConnection(events::ConnectionFailure reason);
    [[nodiscard]] std::unique_ptr<engine::core::IScene> makeMainMenu();
    [[nodiscard]] std::unique_ptr<engine::core::IScene> makeGame();
    [[nodiscard]] std::unique_ptr<engine::core::IScene> makeRespawn();
    [[nodiscard]] std::unique_ptr<engine::core::IScene> makeGameOver();
    [[nodiscard]] std::unique_ptr<engine::core::IScene> makeNotice();

    AppServices _services;
    engine::ecs::UISceneContext _context;
    engine::core::SceneManager _scenes;
    scenes::SceneRouter _router;
    hud::StatsOverlay _stats;
    std::shared_ptr<client::net::ClientSession> _session;
    GameScene *_game = nullptr;
    std::string _notice;
    bool _connecting = false;
    std::array<engine::core::EventBus::ListenerId, 4> _listeners{};
};

} // namespace client::flow

#endif /* !CLIENTAPP_HPP_ */
