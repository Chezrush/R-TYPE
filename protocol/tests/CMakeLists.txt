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

rtype_add_test(byte_stream_tests
    ByteStreamTests.cpp
)

target_link_libraries(byte_stream_tests
    PRIVATE
        rtype_protocol
)

set_tests_properties(byte_stream_tests PROPERTIES TIMEOUT 10)

rtype_add_test(numeric_tests
    NumericTests.cpp
)

target_link_libraries(numeric_tests
    PRIVATE
        rtype_protocol
)

set_tests_properties(numeric_tests PROPERTIES TIMEOUT 10)

rtype_add_test(message_codec_tests
    MessageCodecTests.cpp
)

target_link_libraries(message_codec_tests
    PRIVATE
        rtype_protocol
)

set_tests_properties(message_codec_tests PROPERTIES TIMEOUT 10)

rtype_add_test(receive_window_tests
    ReceiveWindowTests.cpp
)

target_link_libraries(receive_window_tests
    PRIVATE
        rtype_protocol
)

set_tests_properties(receive_window_tests PROPERTIES TIMEOUT 10)

rtype_add_test(reliable_channel_tests
    ReliableChannelTests.cpp
)

target_link_libraries(reliable_channel_tests
    PRIVATE
        rtype_protocol
)

set_tests_properties(reliable_channel_tests PROPERTIES TIMEOUT 10)

rtype_add_test(security_tests
    SecurityTests.cpp
)

target_link_libraries(security_tests
    PRIVATE
        rtype_protocol
)

set_tests_properties(security_tests PROPERTIES TIMEOUT 10)

rtype_add_test(connection_tests
    ConnectionTests.cpp
)

target_link_libraries(connection_tests
    PRIVATE
        rtype_protocol
)

set_tests_properties(connection_tests PROPERTIES TIMEOUT 20)
