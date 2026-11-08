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

rtype_add_test(disconnection_tests
    DisconnectionTests.cpp
)

target_link_libraries(disconnection_tests
    PRIVATE
        rtype_server_core
)

set_tests_properties(disconnection_tests PROPERTIES TIMEOUT 10)

rtype_add_test(gateway_tests
    GatewayTests.cpp
)

target_link_libraries(gateway_tests
    PRIVATE
        rtype_server_core
)

set_tests_properties(gateway_tests PROPERTIES TIMEOUT 10)

rtype_add_test(network_tests
    NetworkTests.cpp
)

target_link_libraries(network_tests
    PRIVATE
        rtype_server_core
)

set_tests_properties(network_tests PROPERTIES TIMEOUT 30)

rtype_add_test(game_bridge_tests
    GameBridgeTests.cpp
)

target_link_libraries(game_bridge_tests
    PRIVATE
        rtype_server_core
)

set_tests_properties(game_bridge_tests PROPERTIES TIMEOUT 10)

rtype_add_test(local_address_tests
    LocalAddressTests.cpp
)

target_link_libraries(local_address_tests
    PRIVATE
        rtype_server_core
)

set_tests_properties(local_address_tests PROPERTIES TIMEOUT 10)

rtype_add_test(net_stats_tests
    NetStatsTests.cpp
)

target_link_libraries(net_stats_tests
    PRIVATE
        rtype_server_core
)

set_tests_properties(net_stats_tests PROPERTIES TIMEOUT 10)
