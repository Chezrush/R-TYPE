##
## EPITECH PROJECT, 2026
## G-ING-500-PAR-5-1-rtype-13
## File description:
## CMakeLists.txt
##

add_executable(rtype_net_measure
    NetMeasure.cpp
)

target_link_libraries(rtype_net_measure
    PRIVATE
        rtype_server_core
        rtype_warnings
)
