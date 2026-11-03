/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ConsoleService.hpp
*/

#ifndef CONSOLESERVICE_HPP_
#define CONSOLESERVICE_HPP_

#include <vector>

#include "Client/Console/ConsoleModel.hpp"
#include "Engine/Core/EventBus.hpp"

namespace client::console {

/**
 * @brief Keeps the console content while it is closed, and opens it.
 *
 * It records ConsoleOutput and ConsoleCommandsListed at any time. The key above
 * Tab (RawKeyPressed with KEY_GRAVE) asks for the CONSOLE overlay when the
 * console is closed; the open console closes itself on that key. The listeners
 * are removed on destruction, so the bus must outlive this object.
 */
class ConsoleService {
public:
    explicit ConsoleService(engine::core::EventBus &bus);
    ~ConsoleService();

    ConsoleService(const ConsoleService &) = delete;
    ConsoleService &operator=(const ConsoleService &) = delete;
    ConsoleService(ConsoleService &&) = delete;
    ConsoleService &operator=(ConsoleService &&) = delete;

    [[nodiscard]] ConsoleModel &model() noexcept;
    [[nodiscard]] bool isOpen() const noexcept;
    void setOpen(bool open) noexcept;

private:
    engine::core::EventBus &_bus;
    std::vector<engine::core::EventBus::ListenerId> _listeners;
    ConsoleModel _model;
    bool _open = false;
};

} // namespace client::console

#endif /* !CONSOLESERVICE_HPP_ */
