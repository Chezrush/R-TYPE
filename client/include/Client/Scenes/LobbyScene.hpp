/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** LobbyScene.hpp
*/

#ifndef LOBBYSCENE_HPP_
#define LOBBYSCENE_HPP_

#include <optional>
#include <vector>

#include "Client/Events/LobbyEvents.hpp"
#include "Client/Lobby/ChatService.hpp"
#include "Client/Scenes/SceneRoute.hpp"
#include "Engine/Ecs/Components/UIWidgetEvents.hpp"
#include "Engine/Ecs/UIScene.hpp"

namespace client::scenes {

/** @brief The widgets of the lobby, after the 4 slots. */
enum class LobbyWidget : std::uint8_t {
    TITLE,
    READY,
    START,
    LEAVE,
    CHAT_LOG,
    CHAT_INPUT
};

/**
 * @brief The players of a room, before the game.
 *
 * Every slot shows the player's color, number and shape, nickname and ready
 * state, updated live from LobbyUpdated. Ready publishes ReadyChanged; Start,
 * only for the host once everyone is ready, publishes StartGameRequested. The
 * game starts on GameStateChanged PLAYING. The chat sends on Enter in its
 * field. The chat service must outlive the scene.
 */
class LobbyScene : public engine::ecs::UIScene {
public:
    LobbyScene(engine::ecs::UISceneContext context, lobby::ChatService &chat);

    [[nodiscard]] engine::ecs::Entity part(LobbyWidget which) const;
    [[nodiscard]] engine::ecs::Entity slotTag(std::size_t slot) const;
    [[nodiscard]] engine::ecs::Entity slotText(std::size_t slot) const;
    [[nodiscard]] engine::ecs::Entity slotShape(std::size_t slot) const;

protected:
    void activate() override;

private:
    void buildSlots();
    void buildSlot(std::size_t slot);
    void buildButtons();
    void buildChat();
    void listenToWidgets();
    void listenToServer();
    void onButton(const engine::ecs::components::UIButtonClickedEvent &event);
    void onLobby(const events::LobbyUpdated &lobby);
    void syncSlot(std::size_t slot);
    void refreshChat();

    lobby::ChatService &_chat;
    std::optional<events::LobbyUpdated> _lobby;
    std::vector<engine::ecs::Entity> _tags;
    std::vector<engine::ecs::Entity> _texts;
    std::vector<engine::ecs::Entity> _shapes;
    std::vector<engine::ecs::Entity> _parts;
};

/** @brief Gives the router the room browser, the lobby and the in-game chat. */
void addLobbyRoutes(SceneRouter &router,
                    const engine::ecs::UISceneContext &context,
                    lobby::ChatService &chat);

} // namespace client::scenes

#endif /* !LOBBYSCENE_HPP_ */
