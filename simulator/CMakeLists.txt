cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Debug)

project(falafels)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(SimGrid REQUIRED)

include_directories(${SimGrid_INCLUDE_DIR})

set(falafels_files
    src/node/mediator/mediator.hpp
    src/node/mediator/mediator_consumer.cpp
    src/node/mediator/mediator_consumer.hpp
    src/node/mediator/mediator_producer.cpp
    src/node/mediator/mediator_producer.hpp

    src/node/network_managers/full_nm.cpp
    src/node/network_managers/full_nm.hpp
    src/node/network_managers/nm.cpp
    src/node/network_managers/nm.hpp
    src/node/network_managers/ring_nm.cpp
    src/node/network_managers/ring_nm.hpp
    src/node/network_managers/star_nm.cpp
    src/node/network_managers/star_nm.hpp

    src/node/roles/aggregator/aggregator.cpp
    src/node/roles/aggregator/aggregator.hpp
    src/node/roles/aggregator/asynchronous_aggregator.cpp
    src/node/roles/aggregator/asynchronous_aggregator.hpp
    src/node/roles/aggregator/hierarchical_aggregator.cpp
    src/node/roles/aggregator/hierarchical_aggregator.hpp
    src/node/roles/aggregator/simple_aggregator.cpp
    src/node/roles/aggregator/simple_aggregator.hpp

    # src/node/roles/proxy/proxy.cpp
    # src/node/roles/proxy/proxy.hpp

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

    src/dot.cpp
    src/dot.hpp

    src/protocol.cpp
    src/protocol.hpp
)

add_executable(main 
    src/main.cpp
    ${falafels_files}
)

target_link_libraries(main ${SimGrid_LIBRARY})
target_link_libraries(main pugixml)
