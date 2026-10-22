/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ResourceCache.tpp
*/

#ifndef RESOURCECACHE_TPP_
#define RESOURCECACHE_TPP_

#include <utility>

#include "Engine/Resources/ResourceCache.hpp"

namespace engine::resources {

template <typename Resource>
ResourceCache<Resource>::ResourceCache(Loader loader)
    : _loader(std::move(loader))
{
}

template <typename Resource>
const Resource *ResourceCache<Resource>::get(
    const std::filesystem::path &path
)
{
    const std::string key = path.generic_string();
    const auto it = _entries.find(key);

    if (it != _entries.end())
        return it->second.get();
    auto resource = std::make_unique<Resource>();
    if (!_loader(*resource, path))
        resource.reset();
    return _entries.emplace(key, std::move(resource)).first->second.get();
}

template <typename Resource>
std::size_t ResourceCache<Resource>::size() const noexcept
{
    return _entries.size();
}

template <typename Resource>
void ResourceCache<Resource>::clear() noexcept
{
    _entries.clear();
}

} // namespace engine::resources

#endif /* !RESOURCECACHE_TPP_ */
