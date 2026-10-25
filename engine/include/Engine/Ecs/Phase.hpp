/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** Phase.hpp
*/

#ifndef PHASE_HPP_
#define PHASE_HPP_

#include <cstddef>
#include <cstdint>

namespace engine::ecs {

/**
 * @brief Stage of a frame in which a system runs.
 *
 * Phases run in the order of their declaration. They carry no gameplay
 * meaning: the engine only guarantees that every system of a phase has run
 * before the next phase starts.
 */
enum class Phase : std::uint8_t {
    PRE_UPDATE,
    UPDATE,
    POST_UPDATE,
};

/** @brief Number of phases, the size of an array indexed by Phase. */
inline constexpr std::size_t PHASE_COUNT = 3;

/** @return A static, human-readable name of the phase. */
[[nodiscard]] const char *phaseName(Phase phase) noexcept;

} // namespace engine::ecs

#endif /* !PHASE_HPP_ */
