cmake_minimum_required(VERSION 3.28.3)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(falafels)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(SimGrid REQUIRED)

include_directories(${SimGrid_INCLUDE_DIR})

set(falafels_files
    src/constants.hpp
    src/protocol.hpp

    src/node/node.hpp

    src/utils/utils.cpp
    src/utils/utils.hpp

    src/node/roles/aggregator.cpp
    src/node/roles/aggregator.hpp
    src/node/roles/proxy.cpp
    src/node/roles/proxy.hpp
    src/node/roles/role.hpp
    src/node/roles/trainer.cpp
    src/node/roles/trainer.hpp

    src/node/network_managers/centralized_nm.cpp
    src/node/network_managers/centralized_nm.hpp
    src/node/network_managers/decentralized_nm.cpp
    src/node/network_managers/decentralized_nm.hpp
    src/node/network_managers/nm.hpp
)

add_executable(main 
    src/main.cpp
    ${falafels_files}
)

target_link_libraries(main ${SimGrid_LIBRARY})
