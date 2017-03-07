find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(UnitTests "")
target_compile_options(UnitTests PUBLIC ${COMPILE_OPTIONS})
target_link_libraries(UnitTests Game ${GTEST_LIBRARIES} ${GTEST_MAIN_LIBRARIES})

target_sources(UnitTests PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/ExampleTest.cpp
    ${CMAKE_CURRENT_LIST_DIR}/OtherTest.cpp
)
add_test(NAME Example COMMAND UnitTests --gtest_filter=Example*)
add_test(NAME Other COMMAND UnitTests --gtest_filter=Other*)
