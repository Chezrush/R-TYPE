/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** LobbyScenes.cpp
*/

#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <string>
#include <vector>

#include "Client/Events/GameEvents.hpp"
#include "Client/Events/LobbyEvents.hpp"
#include "Client/Events/OverlayEvents.hpp"
#include "Client/Events/SettingsEvents.hpp"
#include "Client/Hud/HudView.hpp"
#include "Client/Lobby/ChatService.hpp"
#include "Client/Lobby/LobbyText.hpp"
#include "Client/Scenes/ChatOverlay.hpp"
#include "Client/Scenes/LobbyScene.hpp"
#include "Client/Scenes/RoomBrowserScene.hpp"
#include "Engine/Ecs/Components/UIShape.hpp"
#include "Engine/Ecs/Components/UIToggle.hpp"
#include "SceneFixture.hpp"

using namespace client::events;
using client::lobby::ChatService;
using client::scenes::BrowserWidget;
using client::scenes::ChatOverlay;
using client::scenes::LobbyScene;
using client::scenes::LobbyWidget;
using client::scenes::RoomBrowserScene;
using client::scenes::SceneRoute;
using client::scenes::SceneRouter;
using engine::ecs::UIKey;
using engine::ecs::components::UIButton;
using engine::ecs::components::UIButtonState;
using engine::ecs::components::UIPanel;
using engine::ecs::components::UIScrollList;
using engine::ecs::components::UIShape;
using engine::ecs::components::UIText;
using engine::ecs::components::UITextInput;

namespace {

/** @brief Stands for the game scene, so that T can open the chat. */
class FakeGame : public engine::ecs::UIScene {
public:
    using UIScene::UIScene;

protected:
    void activate() override {}
};

struct LobbyFixture : SceneFixture {
    LobbyFixture()
    {
        client::scenes::addLobbyRoutes(router, context(), chat);
        bus.subscribe<RoomListRequested>(
            [this](const RoomListRequested &) { ++listRequests; });
        bus.subscribe<JoinRoomRequested>(
            [this](const JoinRoomRequested &e) { joins.push_back(e.roomId); });
        bus.subscribe<CreateRoomRequested>(
            [this](const CreateRoomRequested &e) { creates.push_back(e); });
        bus.subscribe<ReadyChanged>(
            [this](const ReadyChanged &e) { readies.push_back(e.ready); });
        bus.subscribe<StartGameRequested>(
            [this](const StartGameRequested &) { ++starts; });
        bus.subscribe<ChatMessageSent>(
            [this](const ChatMessageSent &e) { sent.push_back(e.text); });
        bus.subscribe<TextEntryStarted>(
            [this](const TextEntryStarted &) { ++entryStarted; });
        bus.subscribe<TextEntryEnded>(
            [this](const TextEntryEnded &) { ++entryEnded; });
    }

    ~LobbyFixture()
    {
        scenes.clear();
    }

    LobbyFixture(const LobbyFixture &) = delete;
    LobbyFixture &operator=(const LobbyFixture &) = delete;
    LobbyFixture(LobbyFixture &&) = delete;
    LobbyFixture &operator=(LobbyFixture &&) = delete;

    void go(SceneRoute target)
    {
        bus.publish(client::scenes::NavigationRequested{target});
        router.apply();
        frame();
    }

    void step()
    {
        layout.update(registry);
        scenes.handleInput();
        router.apply();
    }

    template <typename Scene>
    Scene *top()
    {
        return dynamic_cast<Scene *>(&scenes.currentScene());
    }

    std::string text(engine::ecs::Entity entity) const
    {
        return registry.getComponents<UIText>().get(entity)->text;
    }

    void typeIn(engine::ecs::Entity field, const std::u32string &characters)
    {
        click(field);
        type(characters);
    }

    ChatService chat{bus, scenes, [this] { return seconds; }};
    SceneRouter router{scenes, bus};
    double seconds = 0.0;
    int listRequests = 0;
    std::vector<std::uint32_t> joins;
    std::vector<CreateRoomRequested> creates;
    std::vector<bool> readies;
    int starts = 0;
    std::vector<std::string> sent;
    int entryStarted = 0;
    int entryEnded = 0;
};

RoomListReceived someRooms()
{
    return {{{7, "Bob's room", 2, 4, RoomStatus::WAITING},
             {9, "Full house", 2, 2, RoomStatus::WAITING}}};
}

LobbyUpdated lobbyWith(bool bobReady, std::uint8_t local)
{
    LobbyUpdated lobby;

    lobby.roomName = "Bob's room";
    lobby.slots[0] = {true, "Bob", true};
    lobby.slots[2] = {true, "Ann", bobReady};
    lobby.localPlayer = local;
    lobby.hostPlayer = 0;
    return lobby;
}

void selectRow(LobbyFixture &fixture, std::size_t row)
{
    auto *browser = fixture.top<RoomBrowserScene>();
    fixture.click(browser->part(BrowserWidget::LIST));
    fixture.registry.getComponents<UIScrollList>()
        .get(browser->part(BrowserWidget::LIST))
        ->selected = row;
}

} // namespace

TEST_CASE("the browser asks for the rooms and lists them",
          "[client][scenes][lobby]")
{
    LobbyFixture fixture;
    fixture.go(SceneRoute::ROOMS);
    REQUIRE(fixture.listRequests == 1);

    fixture.bus.publish(someRooms());

    const auto &list = *fixture.registry.getComponents<UIScrollList>().get(
        fixture.top<RoomBrowserScene>()->part(BrowserWidget::LIST));
    REQUIRE(list.rows == std::vector<std::string>{"Bob's room   2/4   Waiting",
                                                  "Full house   2/2   Full"});
    fixture.click(
        fixture.top<RoomBrowserScene>()->part(BrowserWidget::REFRESH));
    REQUIRE(fixture.listRequests == 2);
}

TEST_CASE("joining a room goes to the lobby", "[client][scenes][lobby]")
{
    LobbyFixture fixture;
    fixture.go(SceneRoute::ROOMS);
    fixture.bus.publish(someRooms());
    selectRow(fixture, 0);

    fixture.click(fixture.top<RoomBrowserScene>()->part(BrowserWidget::JOIN));
    REQUIRE(fixture.joins == std::vector<std::uint32_t>{7});
    REQUIRE(fixture.top<RoomBrowserScene>()->isWaiting());

    fixture.bus.publish(lobbyWith(false, 2));
    fixture.router.apply();
    REQUIRE(fixture.top<LobbyScene>() != nullptr);
}

TEST_CASE("join failures are shown clearly", "[client][scenes][lobby]")
{
    LobbyFixture fixture;
    fixture.go(SceneRoute::ROOMS);
    fixture.bus.publish(someRooms());
    auto *browser = fixture.top<RoomBrowserScene>();
    const auto status = browser->part(BrowserWidget::STATUS);

    fixture.click(browser->part(BrowserWidget::JOIN));
    REQUIRE(fixture.text(status) == "Select a room first.");
    selectRow(fixture, 1);
    fixture.click(browser->part(BrowserWidget::JOIN));
    REQUIRE(fixture.text(status).find("full") != std::string::npos);
    REQUIRE(fixture.joins.empty());

    selectRow(fixture, 0);
    fixture.click(browser->part(BrowserWidget::JOIN));
    fixture.bus.publish(JoinFailed{JoinFailure::ALREADY_STARTED});
    REQUIRE(fixture.text(status) ==
            client::lobby::joinFailureMessage(JoinFailure::ALREADY_STARTED));
    REQUIRE_FALSE(browser->isWaiting());
    REQUIRE(fixture.registry.getComponents<UIText>().get(status)->colorRole ==
            engine::ecs::UIColorRole::ERROR_TEXT);
}

TEST_CASE("a lobby update without a request is ignored by the browser",
          "[client][scenes][lobby]")
{
    LobbyFixture fixture;
    fixture.go(SceneRoute::ROOMS);

    fixture.bus.publish(lobbyWith(false, 2));
    fixture.router.apply();

    REQUIRE(fixture.top<RoomBrowserScene>() != nullptr);
}

TEST_CASE("creating a room checks the name and the size",
          "[client][scenes][lobby]")
{
    LobbyFixture fixture;
    fixture.go(SceneRoute::ROOMS);
    auto *browser = fixture.top<RoomBrowserScene>();

    fixture.click(browser->part(BrowserWidget::CREATE));
    REQUIRE(fixture.creates.empty());
    fixture.typeIn(browser->part(BrowserWidget::NAME), U"Duo");
    fixture.click(browser->part(BrowserWidget::MAX_PLAYERS));
    fixture.key(UIKey::UP);
    fixture.key(UIKey::UP);
    fixture.key(UIKey::ENTER);
    fixture.click(browser->part(BrowserWidget::CREATE));

    REQUIRE(fixture.creates.size() == 1);
    REQUIRE(fixture.creates[0].name == "Duo");
    REQUIRE(fixture.creates[0].maxPlayers == 2);
}

TEST_CASE("Back leaves the server", "[client][scenes][lobby]")
{
    LobbyFixture fixture;
    int disconnects = 0;
    fixture.bus.subscribe<DisconnectRequested>(
        [&disconnects](const auto &) { ++disconnects; });
    fixture.go(SceneRoute::ROOMS);

    fixture.click(fixture.top<RoomBrowserScene>()->part(BrowserWidget::BACK));

    REQUIRE(disconnects == 1);
    REQUIRE(fixture.route == SceneRoute::MAIN_MENU);
}

TEST_CASE("lobby slots update live", "[client][scenes][lobby]")
{
    LobbyFixture fixture;
    fixture.go(SceneRoute::LOBBY);
    auto *lobby = fixture.top<LobbyScene>();
    REQUIRE(fixture.text(lobby->slotText(2)) == "Free");

    fixture.bus.publish(lobbyWith(false, 2));
    REQUIRE(fixture.text(lobby->part(LobbyWidget::TITLE)) == "Bob's room");
    REQUIRE(fixture.text(lobby->slotText(0)) == "Bob   Ready   (host)");
    REQUIRE(fixture.text(lobby->slotText(2)) == "Ann   Not ready   (you)");
    REQUIRE(fixture.registry.getComponents<UIPanel>()
                .get(lobby->slotTag(2))
                ->colorRole == engine::ecs::UIColorRole::PLAYER_3);

    LobbyUpdated left = lobbyWith(false, 2);
    left.slots[0] = {};
    fixture.bus.publish(left);
    REQUIRE(fixture.text(lobby->slotText(0)) == "Free");
    REQUIRE(fixture.registry.getComponents<UIPanel>()
                .get(lobby->slotTag(0))
                ->colorRole == engine::ecs::UIColorRole::NONE);
}

TEST_CASE("each slot has its color, number and shape",
          "[client][scenes][lobby]")
{
    LobbyFixture fixture;
    fixture.go(SceneRoute::LOBBY);
    auto *lobby = fixture.top<LobbyScene>();

    for (std::size_t slot = 0; slot < LOBBY_SLOTS; ++slot) {
        REQUIRE(fixture.text(lobby->slotTag(slot)) ==
                "P" + std::to_string(slot + 1));
        REQUIRE(fixture.registry.getComponents<UIShape>()
                    .get(lobby->slotShape(slot))
                    ->kind == client::hud::HudView::playerShape(slot));
    }
}

TEST_CASE("Ready is sent and follows the server", "[client][scenes][lobby]")
{
    LobbyFixture fixture;
    fixture.go(SceneRoute::LOBBY);
    fixture.bus.publish(lobbyWith(false, 2));
    auto *lobby = fixture.top<LobbyScene>();

    fixture.key(UIKey::ENTER);
    REQUIRE(fixture.readies == std::vector<bool>{true});

    fixture.bus.publish(lobbyWith(false, 2));
    REQUIRE_FALSE(fixture.registry.getComponents<engine::ecs::UIToggle>()
                      .get(lobby->part(LobbyWidget::READY))
                      ->checked);
}

TEST_CASE("only the host can start, once everyone is ready",
          "[client][scenes][lobby]")
{
    LobbyFixture fixture;
    fixture.go(SceneRoute::LOBBY);
    auto *lobby = fixture.top<LobbyScene>();
    const auto start = lobby->part(LobbyWidget::START);
    const auto state = [&fixture, start] {
        return fixture.registry.getComponents<UIButton>().get(start)->state;
    };

    fixture.bus.publish(lobbyWith(true, 2));
    REQUIRE(state() == UIButtonState::DISABLED);
    fixture.bus.publish(lobbyWith(false, 0));
    REQUIRE(state() == UIButtonState::DISABLED);
    fixture.click(start);
    REQUIRE(fixture.starts == 0);

    fixture.bus.publish(lobbyWith(true, 0));
    fixture.click(start);
    REQUIRE(fixture.starts == 1);
}

TEST_CASE("the game starts from the server, and Leave goes back",
          "[client][scenes][lobby]")
{
    LobbyFixture fixture;
    int leaves = 0;
    fixture.bus.subscribe<LeaveRoomRequested>(
        [&leaves](const auto &) { ++leaves; });
    fixture.go(SceneRoute::LOBBY);
    const int before = fixture.navigations;

    fixture.bus.publish(GameStateChanged{GamePhase::WAITING, 1});
    REQUIRE(fixture.navigations == before);
    fixture.bus.publish(GameStateChanged{GamePhase::PLAYING, 1});
    REQUIRE(fixture.route == SceneRoute::GAME);

    fixture.click(fixture.top<LobbyScene>()->part(LobbyWidget::LEAVE));
    REQUIRE(leaves == 1);
    REQUIRE(fixture.route == SceneRoute::ROOMS);
}

TEST_CASE("the lobby chat sends on Enter and shows the replies",
          "[client][scenes][lobby]")
{
    LobbyFixture fixture;
    fixture.go(SceneRoute::LOBBY);
    auto *lobby = fixture.top<LobbyScene>();
    fixture.seconds = 65.0;

    fixture.typeIn(lobby->part(LobbyWidget::CHAT_INPUT), U"gl hf");
    fixture.key(UIKey::ENTER);
    fixture.bus.publish(ChatMessageReceived{2, "Ann", "gl hf"});

    REQUIRE(fixture.sent == std::vector<std::string>{"gl hf"});
    REQUIRE(fixture.registry.getComponents<UITextInput>()
                .get(lobby->part(LobbyWidget::CHAT_INPUT))
                ->text.empty());
    REQUIRE(fixture.registry.getComponents<UIScrollList>()
                .get(lobby->part(LobbyWidget::CHAT_LOG))
                ->rows.back() == "[01:05] Ann: gl hf");
    REQUIRE(fixture.readies.empty());
}

TEST_CASE("the mute command hides a player without sending anything",
          "[client][scenes][lobby]")
{
    LobbyFixture fixture;
    fixture.go(SceneRoute::LOBBY);
    auto *lobby = fixture.top<LobbyScene>();

    fixture.typeIn(lobby->part(LobbyWidget::CHAT_INPUT), U"/mute 3");
    fixture.key(UIKey::ENTER);
    fixture.bus.publish(ChatMessageReceived{2, "Ann", "spam"});

    REQUIRE(fixture.sent.empty());
    const auto &rows = fixture.registry.getComponents<UIScrollList>()
                           .get(lobby->part(LobbyWidget::CHAT_LOG))
                           ->rows;
    REQUIRE(rows == std::vector<std::string>{"[00:00] * Muted P3"});
}

TEST_CASE("T opens the chat in game only", "[client][scenes][chat]")
{
    LobbyFixture fixture;
    fixture.scenes.pushScene(std::make_unique<FakeGame>(fixture.context()));

    fixture.bus.publish(RawKeyPressed{client::settings::KEY_T});
    fixture.router.apply();
    REQUIRE(fixture.top<ChatOverlay>() == nullptr);

    fixture.chat.setGameRunning(true);
    fixture.bus.publish(RawKeyPressed{client::settings::KEY_T});
    fixture.router.apply();
    REQUIRE(fixture.top<ChatOverlay>() != nullptr);
    REQUIRE(fixture.entryStarted == 1);

    fixture.bus.publish(RawKeyPressed{client::settings::KEY_T});
    fixture.router.apply();
    REQUIRE(fixture.scenes.size() == 2);
}

TEST_CASE("the in-game chat sends, closes and frees the keys",
          "[client][scenes][chat]")
{
    LobbyFixture fixture;
    fixture.scenes.pushScene(std::make_unique<FakeGame>(fixture.context()));
    fixture.chat.setGameRunning(true);
    fixture.bus.publish(RawKeyPressed{client::settings::KEY_T});
    fixture.router.apply();

    fixture.type(U"incoming!");
    fixture.interaction.onKeyPressed(UIKey::ENTER);
    fixture.step();

    REQUIRE(fixture.sent == std::vector<std::string>{"incoming!"});
    REQUIRE(fixture.top<FakeGame>() != nullptr);
    REQUIRE(fixture.entryEnded == 1);
}

TEST_CASE("Escape closes the in-game chat and nothing else",
          "[client][scenes][chat]")
{
    LobbyFixture fixture;
    int cancels = 0;
    fixture.bus.subscribe<engine::ecs::UICancelEvent>(
        [&cancels](const auto &) { ++cancels; });
    fixture.scenes.pushScene(std::make_unique<FakeGame>(fixture.context()));
    fixture.chat.setGameRunning(true);
    fixture.bus.publish(RawKeyPressed{client::settings::KEY_T});
    fixture.router.apply();

    fixture.interaction.onKeyPressed(UIKey::ESCAPE);
    fixture.step();

    REQUIRE(fixture.top<FakeGame>() != nullptr);
    REQUIRE(cancels == 0);
    REQUIRE(fixture.sent.empty());
}

TEST_CASE("messages received while closed show on opening",
          "[client][scenes][chat]")
{
    LobbyFixture fixture;
    fixture.scenes.pushScene(std::make_unique<FakeGame>(fixture.context()));
    fixture.chat.setGameRunning(true);
    fixture.bus.publish(ChatMessageReceived{0, "Bob", "behind you"});

    fixture.bus.publish(RawKeyPressed{client::settings::KEY_T});
    fixture.router.apply();

    const auto *chat = fixture.top<ChatOverlay>();
    REQUIRE(fixture.registry.getComponents<UIScrollList>()
                .get(*chat->logList())
                ->rows.back() == "[00:00] Bob: behind you");
}
