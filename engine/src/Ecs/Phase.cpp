/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Phase.cpp
*/

#include "Engine/Ecs/Phase.hpp"

namespace engine::ecs {

const char *phaseName(Phase phase) noexcept
{
    switch (phase) {
        case Phase::PRE_UPDATE:
            return "pre-update";
        case Phase::UPDATE:
            return "update";
        case Phase::POST_UPDATE:
            return "post-update";
    }
    return "unknown";
}

} // namespace engine::ecs
