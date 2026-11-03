/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** RoomBrowserScene.hpp
*/

#ifndef ROOMBROWSERSCENE_HPP_
#define ROOMBROWSERSCENE_HPP_

#include <optional>
#include <string>
#include <vector>

#include "Client/Events/LobbyEvents.hpp"
#include "Client/Scenes/SceneRoute.hpp"
#include "Engine/Ecs/Components/UIWidgetEvents.hpp"
#include "Engine/Ecs/UIScene.hpp"

namespace client::scenes {

/** @brief The widgets of the room browser. */
enum class BrowserWidget : std::uint8_t {
    LIST,
    REFRESH,
    JOIN,
    BACK,
    NAME,
    MAX_PLAYERS,
    CREATE,
    STATUS
};

/**
 * @brief List of the server's rooms, and a form to create one.
 *
 * It asks for the list on enter and on Refresh. Join and Create publish their
 * request and wait: LobbyUpdated goes to the lobby, JoinFailed shows why. Back
 * leaves the server for the main menu.
 */
class RoomBrowserScene : public engine::ecs::UIScene {
public:
    explicit RoomBrowserScene(engine::ecs::UISceneContext context);

    [[nodiscard]] engine::ecs::Entity part(BrowserWidget which) const;
    [[nodiscard]] bool isWaiting() const noexcept;

protected:
    void activate() override;

private:
    void buildList();
    void buildForm();
    void buildActions();
    void listenToWidgets();
    void listenToServer();
    void onButton(const engine::ecs::components::UIButtonClickedEvent &event);
    void onRooms(const events::RoomListReceived &list);
    void join();
    void create();
    void showStatus(const std::string &text, bool error);

    std::vector<events::RoomInfo> _rooms;
    std::vector<engine::ecs::Entity> _parts;
    bool _waiting = false;
};

} // namespace client::scenes

#endif /* !ROOMBROWSERSCENE_HPP_ */
