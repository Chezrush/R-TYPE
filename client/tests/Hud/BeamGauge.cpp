/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** BeamGauge.cpp
*/

#include "Client/Hud/BeamGauge.hpp"

#include <catch2/catch_test_macros.hpp>

using client::hud::BeamGauge;

TEST_CASE("the beam is empty when idle", "[client][hud][beam]")
{
    BeamGauge beam(2.0f);

    beam.update(1.0f, false);
    REQUIRE(beam.charge() == 0.0f);
}

TEST_CASE("the beam fills while shoot is held", "[client][hud][beam]")
{
    BeamGauge beam(2.0f);

    beam.update(0.5f, true);
    REQUIRE(beam.charge() == 0.25f);
    beam.update(1.0f, true);
    REQUIRE(beam.charge() == 0.75f);
    REQUIRE_FALSE(beam.isFull());
    beam.update(5.0f, true);
    REQUIRE(beam.charge() == 1.0f);
    REQUIRE(beam.isFull());
}

TEST_CASE("releasing shoot resets the beam", "[client][hud][beam]")
{
    BeamGauge beam(1.0f);

    beam.update(0.5f, true);
    beam.update(0.016f, false);
    REQUIRE(beam.charge() == 0.0f);
}

TEST_CASE("a zero charge time fills at once", "[client][hud][beam]")
{
    BeamGauge beam(0.0f);

    beam.update(0.0f, true);
    REQUIRE(beam.isFull());
}
