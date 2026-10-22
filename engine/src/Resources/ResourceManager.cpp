/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ResourceManager.cpp
*/

#include "Engine/Resources/ResourceManager.hpp"

#include <utility>

namespace engine::resources {

namespace {

bool loadFont(sf::Font &font, const std::filesystem::path &path)
{
    return font.openFromFile(path);
}

bool loadTexture(sf::Texture &texture, const std::filesystem::path &path)
{
    return texture.loadFromFile(path);
}

} // namespace

ResourceManager::ResourceManager(std::filesystem::path assetRoot)
    : _assetRoot(std::move(assetRoot)),
      _fonts(loadFont),
      _textures(loadTexture)
{}

const sf::Font *ResourceManager::font(const std::string &id)
{
    if (id.empty())
        return nullptr;
    return _fonts.get(_assetRoot / id);
}

const sf::Texture *ResourceManager::texture(const std::string &id)
{
    if (id.empty())
        return nullptr;
    return _textures.get(_assetRoot / id);
}

const std::filesystem::path &ResourceManager::assetRoot() const noexcept
{
    return _assetRoot;
}

void ResourceManager::clear() noexcept
{
    _fonts.clear();
    _textures.clear();
}

} // namespace engine::resources
