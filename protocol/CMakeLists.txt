##
## EPITECH PROJECT, 2026
## G-ING-500-PAR-5-1-rtype-13
## File description:
## CMakeLists.txt
##

add_library(rtype_protocol STATIC
    src/ByteOrder.cpp
    src/PacketBuilder.cpp
    src/PacketReader.cpp
)

target_include_directories(rtype_protocol PUBLIC include)
target_compile_features(rtype_protocol PUBLIC cxx_std_20)
target_link_libraries(rtype_protocol PRIVATE rtype_warnings)

if(RTYPE_BUILD_TESTS)
    add_subdirectory(tests)
endif()
