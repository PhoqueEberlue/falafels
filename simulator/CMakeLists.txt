cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Debug)

project(falafels-simulator)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

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

add_executable(falafels-simulator 
    src/main.cpp
    ${falafels_files}
)

target_link_libraries(falafels-simulator ${SimGrid_LIBRARY})
target_link_libraries(falafels-simulator pugixml)

# Specify the installation directories
set(INSTALL_BIN_DIR bin)
set(INSTALL_LIB_DIR lib)
set(INSTALL_INCLUDE_DIR include)

# Install the executable
install(TARGETS falafels-simulator
    RUNTIME DESTINATION ${INSTALL_BIN_DIR}
)

## IDK I can't manage to install simgrid
# Install the versioned SimGrid library
## install(FILES ${SimGrid_LIBRARY}
##     DESTINATION ${INSTALL_LIB_DIR}
##     RENAME libsimgrid.so.3.35.1
## )
## 
## # Create symbolic links for the versioned library
## install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink libsimgrid.so.3.35.1 libsimgrid.so.3)")
## install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink libsimgrid.so.3 libsimgrid.so)")
## # install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink libsimgrid.so.3.35.1 libsimgrid.so)")
## 
## # If pugixml is an external library and you have it as a separate library file, install it as well
## # For example, if you have libpugixml.so or pugixml.lib
## # You can specify the full path to the pugixml library file
## # install(FILES /path/to/pugixml
## #     DESTINATION ${INSTALL_LIB_DIR}
## # )
## 
## # Adjust the RPATH for the executable so it can find the installed libraries
## set_target_properties(falafels-simulator PROPERTIES
##     INSTALL_RPATH "\$ORIGIN/../${INSTALL_LIB_DIR}"
##     BUILD_WITH_INSTALL_RPATH TRUE
## )
## 
## # Optionally, you can also install headers or other necessary files
## # install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
## #     DESTINATION ${INSTALL_INCLUDE_DIR}
## # )
