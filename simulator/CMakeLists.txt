cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Debug)

project(falafels)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

# ----------------------------------- Linking dmlogic with the simulator -----------------------------------
set(DMLOGIC_PATH ${CMAKE_SOURCE_DIR}/../dmlogic)
set(CARGO_MANIFEST ${DMLOGIC_PATH}/Cargo.toml)
set(CARGO_TARGET_DIR ${DMLOGIC_PATH}/target)

set(DMLOGIC_SOURCE_FILE ${DMLOGIC_PATH}/src/bridge.rs)
set(DMLOGIC_BRIDGE_CPP ${CARGO_TARGET_DIR}/cxxbridge/blobstore/src/bridge.rs.cc)
set(DMLOGIC_LIB ${CARGO_TARGET_DIR}/debug/${CMAKE_STATIC_LIBRARY_PREFIX}dmlogic${CMAKE_STATIC_LIBRARY_SUFFIX})

# Add a custom command that builds the rust crate and generates C++ bridge code
add_custom_command(
        OUTPUT ${DMLOGIC_BRIDGE_CPP} ${DMLOGIC_LIB}
        COMMAND cargo build --manifest-path ${CARGO_MANIFEST}
        DEPENDS ${BLOBSTORE_SOURCE_FILE}
        USES_TERMINAL
        COMMENT "Running cargo..."
)
# ----------------------------------------------------------------------------------------------------------

find_package(SimGrid REQUIRED)

include_directories(${SimGrid_INCLUDE_DIR})

set(falafels_files
    src/node/mediator/mediator.hpp
    src/node/mediator/mediator_consumer.cpp
    src/node/mediator/mediator_consumer.hpp
    src/node/mediator/mediator_producer.cpp
    src/node/mediator/mediator_producer.hpp

    # src/node/network_managers/full_nm.cpp
    # src/node/network_managers/full_nm.hpp
    src/node/network_managers/hierarchical_nm.cpp
    src/node/network_managers/hierarchical_nm.hpp
    src/node/network_managers/nm.cpp
    src/node/network_managers/nm.hpp
    src/node/network_managers/ring_bi_nm.cpp
    src/node/network_managers/ring_bi_nm.hpp
    src/node/network_managers/ring_uni_nm.cpp
    src/node/network_managers/ring_uni_nm.hpp
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

target_link_libraries(main ${SimGrid_LIBRARY} ${DMLOGIC_LIB} pugixml)

# Include bridge
target_include_directories(
        main 
        PRIVATE
        bridge_include/
        ${CARGO_TARGET_DIR}/cxxbridge/dmlogic/src/
)

# Add cargo target dir, but I don't know why to be fair
set_target_properties(
        main 
        PROPERTIES ADDITIONAL_CLEAN_FILES ${CARGO_TARGET_DIR}
)

# Windows-only configuration
if(WIN32)
    target_link_libraries(main userenv ws2_32 bcrypt)
    set_target_properties(
            main 
            PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
            RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}
            RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}
    )
endif()
