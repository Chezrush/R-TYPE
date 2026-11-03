/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ConnectScene.hpp
*/

#ifndef CONNECTSCENE_HPP_
#define CONNECTSCENE_HPP_

#include <cstddef>
#include <string>
#include <vector>

#include "Client/Events/ConnectionEvents.hpp"
#include "Client/Scenes/ConnectForm.hpp"
#include "Engine/Ecs/Components/UITextInput.hpp"
#include "Engine/Ecs/Components/UIWidgetEvents.hpp"
#include "Engine/Ecs/UIScene.hpp"

namespace client::scenes {

/**
 * @brief Connection form: address, port and nickname.
 *
 * It never calls network code. "Connect" (or Enter in a field) publishes
 * ConnectRequested after checking the form; "Back" (or Escape) goes back to the
 * main menu and publishes ConnectCancelled if a connection was in progress.
 * Then it reacts to the network layer's events: ConnectionStarted shows the
 * progress, WelcomeReceived goes to the game, ConnectionFailed shows why and
 * gives the form back. Those events are ignored unless the scene is waiting.
 */
class ConnectScene : public engine::ecs::UIScene {
public:
    explicit ConnectScene(engine::ecs::UISceneContext context,
                          ConnectForm form = {});

    [[nodiscard]] const ConnectForm &form() const noexcept;
    [[nodiscard]] bool isConnecting() const noexcept;
    [[nodiscard]] const std::string &message() const noexcept;

    [[nodiscard]] engine::ecs::Entity hostField() const;
    [[nodiscard]] engine::ecs::Entity portField() const;
    [[nodiscard]] engine::ecs::Entity nicknameField() const;
    [[nodiscard]] engine::ecs::Entity connectButton() const;
    [[nodiscard]] engine::ecs::Entity backButton() const;
    [[nodiscard]] engine::ecs::Entity statusLabel() const;

protected:
    void activate() override;

private:
    engine::ecs::Entity addField(const std::string &title, float y,
                                 engine::ecs::components::UITextInput input);
    void buildFields();
    void buildActions();
    [[nodiscard]] engine::ecs::Entity part(std::size_t index) const;
    void listenToWidgets();
    void listenToNetwork();
    void onText(const engine::ecs::components::UITextChangedEvent &event);
    void submit();
    void goBack();
    void showError(const std::string &text);
    void showProgress(const std::string &text);
    void setStatus(const std::string &text,
                   engine::ecs::components::UIColor color);
    void setEditable(bool editable);
    void focusOn(engine::ecs::Entity target);

    ConnectForm _form;
    bool _connecting = false;
    std::string _message;
    std::vector<engine::ecs::Entity> _parts;
};

} // namespace client::scenes

#endif /* !CONNECTSCENE_HPP_ */
