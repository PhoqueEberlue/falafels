cmake_minimum_required(VERSION 3.28.3)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(falafels)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(SimGrid REQUIRED)

include_directories(${SimGrid_INCLUDE_DIR})

set(falafels_files
    pugixml/pugiconfig.hpp
    pugixml/pugixml.cpp
    pugixml/pugixml.hpp

    src/node/network_managers/centralized_nm.cpp
    src/node/network_managers/centralized_nm.hpp
    src/node/network_managers/decentralized_nm.cpp
    src/node/network_managers/decentralized_nm.hpp
    src/node/network_managers/nm.hpp

    src/node/roles/aggregator/aggregator.cpp
    src/node/roles/aggregator/aggregator.hpp
    src/node/roles/aggregator/asynchronous_aggregator.cpp
    src/node/roles/aggregator/asynchronous_aggregator.hpp
    src/node/roles/aggregator/simple_aggregator.cpp
    src/node/roles/aggregator/simple_aggregator.hpp

    src/node/roles/proxy/proxy.cpp
    src/node/roles/proxy/proxy.hpp

    src/node/roles/trainer/trainer.cpp
    src/node/roles/trainer/trainer.hpp

    src/node/roles/role.hpp
    
    src/node/node.cpp
    src/node/node.hpp
 
    src/utils/utils.cpp
    src/utils/utils.hpp

    src/config_loader.cpp
    src/config_loader.hpp
    src/constants.hpp
    src/protocol.hpp
)

add_executable(main 
    src/main.cpp
    ${falafels_files}
)

target_link_libraries(main ${SimGrid_LIBRARY})
