cmake_minimum_required(VERSION 3.16)
project(treectl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(treectl
    main/main.cpp
    main/src/btree/btree.cpp
    main/src/cli/orchestrator.cpp
    main/src/utils/metrics.cpp
    main/src/hash/algorithms/murmurhash.cpp
)

target_include_directories(treectl PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(treectl PRIVATE -Wall -Wextra -O2)

if(TREECTL_USE_COMPRESSION)
    target_link_libraries(treectl PRIVATE compressor_lib)
    target_compile_definitions(treectl PRIVATE WITH_COMPRESSION)
    message(STATUS "treectl: compression enabled (linking compressor lib)")
else()
    message(STATUS "treectl: compression disabled")
endif()
