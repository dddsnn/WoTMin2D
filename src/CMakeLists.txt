cmake_minimum_required(VERSION 3.1)
project(WoTMin2D)

if(NOT CMAKE_COMPILER_IS_GNUCXX)
    message(WARNING "Compilers other than g++ are not tested.")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMake/)
set(COMPILE_OPTIONS -std=c++11 -Wall -Wextra -pedantic)

find_package(SDL2 REQUIRED)
find_package(Boost REQUIRED)
include_directories(${SDL2_INCLUDE_DIR} ${Boost_INCLUDE_DIR})

add_library(Game OBJECT "")
target_compile_options(Game PUBLIC ${COMPILE_OPTIONS})

add_executable(WoTMin2D main.cpp $<TARGET_OBJECTS:Game>)
target_compile_options(WoTMin2D PUBLIC ${COMPILE_OPTIONS})
target_link_libraries(WoTMin2D ${SDL2_LIBRARY})

target_sources(Game PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/Battle.cpp
)

# Subdirectories: Use include() instead of add_subdirectory() because
# target_link_library() in subdirectories only works this way.
include(game/CMakeLists.txt)
include(display/CMakeLists.txt)
include(input/CMakeLists.txt)

if(NOT ${CMAKE_BUILD_TYPE} STREQUAL Release)
    enable_testing()
    include(test/CMakeLists.txt)
endif()
