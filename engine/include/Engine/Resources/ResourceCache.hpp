/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ResourceCache.hpp
*/

#ifndef RESOURCECACHE_HPP_
#define RESOURCECACHE_HPP_

#include <cstddef>
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace engine::resources {

/**
 * @brief Loads each resource once and hands out the same instance afterwards.
 *
 * Resources are heap allocated so their address stays valid while the cache
 * grows: an sf::Text keeps a pointer to its sf::Font. A failed load is
 * remembered too, so a missing file is not read again every frame.
 */
template <typename Resource>
class ResourceCache {
public:
    /** @brief Fills the resource from the file, returns false on failure. */
    using Loader =
        std::function<bool(Resource &, const std::filesystem::path &)>;

    explicit ResourceCache(Loader loader);

    /**
     * @return The resource loaded from path, or nullptr if loading failed.
     *
     * Only the first request for a path calls the loader.
     */
    [[nodiscard]] const Resource *get(const std::filesystem::path &path);

    /** @brief Number of paths requested so far, failed ones included. */
    [[nodiscard]] std::size_t size() const noexcept;

    /** @brief Forgets every entry, so the next requests load again. */
    void clear() noexcept;

private:
    Loader _loader;
    std::unordered_map<std::string, std::unique_ptr<Resource>> _entries;
};

} // namespace engine::resources

#include "Engine/Resources/ResourceCache.tpp"

#endif /* !RESOURCECACHE_HPP_ */
