/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** ResourceManager.cpp
*/

#include "Engine/Resources/ResourceManager.hpp"

#include <catch2/catch_test_macros.hpp>

using engine::resources::ResourceManager;

TEST_CASE("missing fonts give nullptr instead of crashing",
          "[resources][manager]")
{
    ResourceManager resources("does/not/exist");

    REQUIRE(resources.font("missing.ttf") == nullptr);
    REQUIRE(resources.font("missing.ttf") == nullptr);
    REQUIRE(resources.font("") == nullptr);
}

TEST_CASE("an empty texture id gives nullptr", "[resources][manager]")
{
    ResourceManager resources;

    REQUIRE(resources.texture("") == nullptr);
}

TEST_CASE("the asset root is kept", "[resources][manager]")
{
    const ResourceManager resources("assets");

    REQUIRE(resources.assetRoot() == "assets");
}
