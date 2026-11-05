##
## EPITECH PROJECT, 2026
## G-ING-500-PAR-5-1-rtype-13
## File description:
## CMakeLists.txt
##

rtype_add_test(packet_reader_tests
    PacketReaderTests.cpp
)

target_link_libraries(packet_reader_tests
    PRIVATE
        rtype_protocol
)

set_tests_properties(packet_reader_tests PROPERTIES TIMEOUT 10)

rtype_add_test(packet_builder_tests
    PacketBuilderTests.cpp
)

target_link_libraries(packet_builder_tests
    PRIVATE
        rtype_protocol
)

set_tests_properties(packet_builder_tests PROPERTIES TIMEOUT 10)
