/*
** EPITECH PROJECT, 2026
** G-ING-500-PAR-5-1-rtype-13
** File description:
** TestSupport.hpp
*/

#ifndef TESTSUPPORT_HPP_
#define TESTSUPPORT_HPP_

#include <iostream>

/**
 * @brief Number of failed checks, shared by every CHECK() of a test binary.
 */
inline int &getTestFailures()
{
    static int failures = 0;

    return failures;
}

/**
 * @brief Records a failure and keeps going when the condition is false.
 */
#define CHECK(cond)                                                            \
    do {                                                                       \
        if (!(cond)) {                                                         \
            std::cerr << __FILE__ << ":" << __LINE__                           \
                      << ": CHECK failed: " #cond << "\n";                     \
            ++getTestFailures();                                               \
        }                                                                      \
    } while (false)

/**
 * @brief Process exit code of a test binary: 0 when every check passed.
 */
#define TEST_RESULT() (getTestFailures() == 0 ? 0 : 1)

#endif /* !TESTSUPPORT_HPP_ */
