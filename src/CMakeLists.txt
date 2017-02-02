cmake_minimum_required(VERSION 3.1)
project(WoTMin2D)

if(NOT CMAKE_COMPILER_IS_GNUCXX)
    message(WARNING "Compilers other than g++ are not tested.")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/CMake/)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

add_executable(WoTMin2D "")
target_compile_options(WoTMin2D PUBLIC -std=c++11 -Wall -Wextra -pedantic)
target_link_libraries(WoTMin2D ${SDL2_LIBRARY})

target_sources(WoTMin2D PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Battle.cpp
)

# Subdirectories: Use include() instead of add_subdirectory() because
# target_link_library() in subdirectories only works this way.
include(game/CMakeLists.txt)
include(display/CMakeLists.txt)
