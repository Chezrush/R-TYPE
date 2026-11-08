##
## EPITECH PROJECT, 2026
## G-ING-500-PAR-5-1-rtype-13
## File description:
## CMakeLists.txt
##

find_package(asio CONFIG REQUIRED)

add_library(rtype_server_core STATIC
    src/ClientActivity.cpp
    src/GameBridge.cpp
    src/Gateway.cpp
    src/InputQueue.cpp
    src/PlayerSessions.cpp
    src/Server.cpp
)

target_include_directories(rtype_server_core
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_features(rtype_server_core PUBLIC cxx_std_20)

target_link_libraries(rtype_server_core
    PRIVATE
        rtype_warnings
    PUBLIC
        rtype_protocol
        rtype::game
        Threads::Threads
        asio::asio
)

add_executable(r-type_server
    src/main.cpp
)

target_link_libraries(r-type_server
    PRIVATE
        rtype_server_core
        rtype_warnings
)

if(RTYPE_BUILD_TESTS)
    add_subdirectory(tests)
endif()