/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ResourceManager.hpp
*/

#ifndef RESOURCEMANAGER_HPP_
#define RESOURCEMANAGER_HPP_

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <filesystem>
#include <string>

#include "Engine/Resources/ResourceCache.hpp"

namespace engine::resources {

/**
 * @brief Cached access to the fonts and textures used by the UI.
 *
 * An id is a path relative to the asset root ("fonts/main.ttf"). Each id is
 * read from disk at most once. The returned pointers stay valid until clear()
 * or the manager is destroyed.
 */
class ResourceManager {
public:
    explicit ResourceManager(std::filesystem::path assetRoot = {});

    /** @return The font, or nullptr if the id is empty or fails to load. */
    [[nodiscard]] const sf::Font *font(const std::string &id);

    /** @return The texture, or nullptr if the id is empty or fails to load. */
    [[nodiscard]] const sf::Texture *texture(const std::string &id);

    /** @brief Directory the ids are resolved against. */
    [[nodiscard]] const std::filesystem::path &assetRoot() const noexcept;

    /** @brief Releases every font and texture, failed ids included. */
    void clear() noexcept;

private:
    std::filesystem::path _assetRoot;
    ResourceCache<sf::Font> _fonts;
    ResourceCache<sf::Texture> _textures;
};

} // namespace engine::resources

#endif /* !RESOURCEMANAGER_HPP_ */
