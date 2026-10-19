/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Version.cpp
*/

#include <catch2/catch_test_macros.hpp>
#include <string>

#include "Engine/Engine.hpp"

TEST_CASE("the engine reports a version", "[engine]")
{
    REQUIRE(std::string(engine::version()) == "0.1.0");
}
