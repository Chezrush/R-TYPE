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

set_tests_properties(server_tests PROPERTIES TIMEOUT 10)

rtype_add_test(message_queue_tests
    MessageQueueTests.cpp
)

target_link_libraries(message_queue_tests
    PRIVATE
        rtype_server_core
)

set_tests_properties(message_queue_tests PROPERTIES TIMEOUT 10)

rtype_add_test(packet_validation_tests
    PacketCheckerTests.cpp
)

target_link_libraries(packet_validation_tests
    PRIVATE
        rtype_server_core
)

set_tests_properties(packet_validation_tests PROPERTIES TIMEOUT 10)
