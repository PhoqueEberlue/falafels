cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_BUILD_TYPE Debug)

project(dml)

set(dml_LIB_HDRS
    include/constants.hpp
    include/i_common.hpp
    include/i_executor.hpp
    include/i_mediator_consumer.hpp
    include/i_mediator_producer.hpp
    include/i_network_manager.hpp
    include/mom.hpp
    include/mom_builder.hpp
    include/protocol.hpp
    include/role.hpp
    include/role_builder.hpp
)

set(dml_LIB_SRCS
#     # src/moms/hierarchical_mom.cpp
#     # src/moms/hierarchical_mom.hpp
#     src/moms/ring_uni_mom.cpp
#     src/moms/ring_uni_mom.hpp
#     src/moms/star_mom.cpp
#     src/moms/star_mom.hpp
# 
#     src/roles/aggregator/aggregator.cpp
#     src/roles/aggregator/aggregator.hpp
#     src/roles/aggregator/asynchronous_aggregator.cpp
#     src/roles/aggregator/asynchronous_aggregator.hpp
#     # src/roles/aggregator/hierarchical_aggregator.cpp
#     # src/roles/aggregator/hierarchical_aggregator.hpp
#     src/roles/aggregator/simple_aggregator.cpp
#     src/roles/aggregator/simple_aggregator.hpp
# 
#     src/roles/trainer/trainer.cpp
#     src/roles/trainer/trainer.hpp
# 
      src/protocol.cpp
)

add_library(dml STATIC ${dml_LIB_SRCS} ${dml_LIB_HDRS})
target_include_directories(dml PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

