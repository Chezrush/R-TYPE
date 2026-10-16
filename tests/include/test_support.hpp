#pragma once

// Minimal test helper: CHECK() records a failure and keeps going, TEST_RESULT() gives the
// process exit code. Swap for GoogleTest / Catch2 whenever the team prefers a framework.

#include <iostream>

inline int& test_failures() {
    static int failures = 0;
    return failures;
}

#define CHECK(cond)                                                                            \
    do {                                                                                       \
        if (!(cond)) {                                                                         \
            std::cerr << __FILE__ << ":" << __LINE__ << ": CHECK failed: " #cond << "\n";    \
            ++test_failures();                                                                 \
        }                                                                                      \
    } while (false)

#define TEST_RESULT() (test_failures() == 0 ? 0 : 1)
