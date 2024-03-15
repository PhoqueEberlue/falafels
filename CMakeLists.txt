cmake_minimum_required(VERSION 3.28.3)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(falafels)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(SimGrid REQUIRED)

include_directories(${SimGrid_INCLUDE_DIR})

set(falafels_files
        src/node.hpp
        src/node.cpp
        src/utils.hpp
        src/utils.cpp
        src/protocol.hpp
)

add_executable(main 
    src/main.cpp
    ${falafels_files}
)

target_link_libraries(main ${SimGrid_LIBRARY})
