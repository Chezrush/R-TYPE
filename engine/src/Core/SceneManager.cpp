/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** SceneManager.cpp
*/

#include "Engine/Core/SceneManager.hpp"

#include <stdexcept>
#include <utility>

namespace engine::core {

SceneManager::~SceneManager()
{
    clear();
}

void SceneManager::pushScene(std::unique_ptr<IScene> scene)
{
    if (!scene) {
        return;
    }
    if (!_scenes.empty()) {
        _scenes.back()->onPause();
    }
    _scenes.push_back(std::move(scene));
    _scenes.back()->onEnter();
}

void SceneManager::popScene()
{
    if (_scenes.empty()) {
        return;
    }
    _scenes.back()->onExit();
    _scenes.pop_back();
    if (!_scenes.empty()) {
        _scenes.back()->onResume();
    }
}

void SceneManager::switchScene(std::unique_ptr<IScene> scene)
{
    clear();
    pushScene(std::move(scene));
}

void SceneManager::clear() noexcept
{
    while (!_scenes.empty()) {
        _scenes.back()->onExit();
        _scenes.pop_back();
    }
}

void SceneManager::update(float dt)
{
    if (!_scenes.empty()) {
        _scenes.back()->update(dt);
    }
}

void SceneManager::render()
{
    if (_scenes.empty()) {
        return;
    }
    std::size_t start = _scenes.size() - 1;
    while (start > 0 && _scenes[start]->isOverlay()) {
        --start;
    }
    for (std::size_t i = start; i < _scenes.size(); ++i) {
        _scenes[i]->render();
    }
}

void SceneManager::handleInput()
{
    if (!_scenes.empty()) {
        _scenes.back()->handleInput();
    }
}

bool SceneManager::empty() const noexcept
{
    return _scenes.empty();
}

std::size_t SceneManager::size() const noexcept
{
    return _scenes.size();
}

IScene &SceneManager::currentScene()
{
    if (_scenes.empty()) {
        throw std::runtime_error("SceneManager: no active scene");
    }
    return *_scenes.back();
}

const IScene &SceneManager::currentScene() const
{
    if (_scenes.empty()) {
        throw std::runtime_error("SceneManager: no active scene");
    }
    return *_scenes.back();
}

} // namespace engine::core
