find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(UnitTests "")
target_compile_options(UnitTests PUBLIC ${COMPILE_OPTIONS})
target_link_libraries(UnitTests Game ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})

target_sources(UnitTests PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/DirectionTest.cpp
    ${CMAKE_CURRENT_LIST_DIR}/VectorTest.cpp
)
add_test(NAME Direction COMMAND UnitTests --gtest_filter=Direction*)
add_test(NAME Vector COMMAND UnitTests --gtest_filter=Vector*)
