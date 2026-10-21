##
## EPITECH PROJECT, 2026
## G-ING-500-PAR-5-1-rtype-13
## File description:
## CMakeLists.txt
##

rtype_add_test(server_tests
    ServerTests.cpp
)

target_link_libraries(server_tests
    PRIVATE
        rtype_server_core
)

# Prevent a shutdown regression from blocking the test runner indefinitely.
set_tests_properties(server_tests PROPERTIES TIMEOUT 10)