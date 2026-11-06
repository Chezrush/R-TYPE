/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Waves.cpp
*/

#include "Rtype/Waves.hpp"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <limits>

using Catch::Approx;
using rtype::game::enemySpeed;
using rtype::game::GameRules;
using rtype::game::levelAfter;

TEST_CASE("the game starts at level 1", "[game][waves]")
{
    REQUIRE(levelAfter(0, GameRules{}) == 1);
}

TEST_CASE("the level goes up every enemiesPerLevel enemies", "[game][waves]")
{
    GameRules rules;
    rules.enemiesPerLevel = 10;

    REQUIRE(levelAfter(9, rules) == 1);
    REQUIRE(levelAfter(10, rules) == 2);
    REQUIRE(levelAfter(19, rules) == 2);
    REQUIRE(levelAfter(20, rules) == 3);
}

TEST_CASE("the level never changes when enemiesPerLevel is 0", "[game][waves]")
{
    GameRules rules;
    rules.enemiesPerLevel = 0;

    REQUIRE(levelAfter(1000000, rules) == 1);
}

TEST_CASE("the level stops at 255", "[game][waves]")
{
    GameRules rules;
    rules.enemiesPerLevel = 1;

    REQUIRE(levelAfter(254, rules) == 255);
    REQUIRE(levelAfter(255, rules) == 255);
    REQUIRE(levelAfter(std::numeric_limits<std::uint32_t>::max(), rules) ==
            255);
}

TEST_CASE("enemies are faster at higher levels", "[game][waves]")
{
    const GameRules rules;

    REQUIRE(enemySpeed(rules, 1) == rules.enemySpeed);
    REQUIRE(enemySpeed(rules, 2) == Approx(rules.enemySpeed * 1.1F));
    REQUIRE(enemySpeed(rules, 11) == Approx(rules.enemySpeed * 2.0F));
}
