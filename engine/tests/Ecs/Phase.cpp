/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Phase.cpp
*/

#include "Engine/Ecs/Phase.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstring>

using engine::ecs::Phase;
using engine::ecs::PHASE_COUNT;
using engine::ecs::phaseName;

TEST_CASE("phases run in a fixed order", "[ecs][phase]")
{
    REQUIRE(Phase::PRE_UPDATE < Phase::UPDATE);
    REQUIRE(Phase::UPDATE < Phase::POST_UPDATE);
}

TEST_CASE("the phase count matches the last phase", "[ecs][phase]")
{
    REQUIRE(static_cast<std::size_t>(Phase::POST_UPDATE) + 1 == PHASE_COUNT);
}

TEST_CASE("every phase has a distinct name", "[ecs][phase]")
{
    REQUIRE(std::strcmp(phaseName(Phase::PRE_UPDATE), "pre-update") == 0);
    REQUIRE(std::strcmp(phaseName(Phase::UPDATE), "update") == 0);
    REQUIRE(std::strcmp(phaseName(Phase::POST_UPDATE), "post-update") == 0);
}

TEST_CASE("an out-of-range phase has a fallback name", "[ecs][phase]")
{
    REQUIRE(std::strcmp(phaseName(static_cast<Phase>(200)), "unknown") == 0);
}
