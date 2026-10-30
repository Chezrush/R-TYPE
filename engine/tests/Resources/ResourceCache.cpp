/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ResourceCache.cpp
*/

#include "Engine/Resources/ResourceCache.hpp"

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <string>

using engine::resources::ResourceCache;

namespace {

struct FakeResource {
    std::string path;
};

} // namespace

TEST_CASE("a resource is loaded once and shared", "[resources][cache]")
{
    int loads = 0;
    ResourceCache<FakeResource> cache(
        [&loads](FakeResource &resource, const std::filesystem::path &path) {
            ++loads;
            resource.path = path.generic_string();
            return true;
        });

    const FakeResource *first = cache.get("fonts/main.ttf");
    const FakeResource *second = cache.get("fonts/main.ttf");

    REQUIRE(first != nullptr);
    REQUIRE(first == second);
    REQUIRE(first->path == "fonts/main.ttf");
    REQUIRE(loads == 1);
    REQUIRE(cache.size() == 1);
}

TEST_CASE("different paths are loaded separately", "[resources][cache]")
{
    int loads = 0;
    ResourceCache<FakeResource> cache(
        [&loads](FakeResource &, const std::filesystem::path &) {
            ++loads;
            return true;
        });

    const FakeResource *font = cache.get("fonts/main.ttf");
    const FakeResource *texture = cache.get("sprites/ship.png");

    REQUIRE(font != texture);
    REQUIRE(loads == 2);
    REQUIRE(cache.size() == 2);
}

TEST_CASE("a failed load returns nullptr and is not retried",
          "[resources][cache]")
{
    int loads = 0;
    ResourceCache<FakeResource> cache(
        [&loads](FakeResource &, const std::filesystem::path &) {
            ++loads;
            return false;
        });

    REQUIRE(cache.get("missing.ttf") == nullptr);
    REQUIRE(cache.get("missing.ttf") == nullptr);
    REQUIRE(loads == 1);
}

TEST_CASE("clear forgets every entry", "[resources][cache]")
{
    int loads = 0;
    ResourceCache<FakeResource> cache(
        [&loads](FakeResource &, const std::filesystem::path &) {
            ++loads;
            return loads > 1;
        });

    REQUIRE(cache.get("late.ttf") == nullptr);
    cache.clear();
    REQUIRE(cache.size() == 0);
    REQUIRE(cache.get("late.ttf") != nullptr);
    REQUIRE(loads == 2);
}
