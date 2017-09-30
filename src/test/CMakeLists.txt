find_package(GTest REQUIRED)
find_package(GMock REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS} ${GMOCK_INCLUDE_DIRS})

add_executable(UnitTests "")
target_compile_options(UnitTests PUBLIC ${COMPILE_OPTIONS})
target_link_libraries(UnitTests
    Game
    ${GTEST_BOTH_LIBRARIES}
    ${GMOCK_BOTH_LIBRARIES}
)

target_sources(UnitTests PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/DirectionTest.cpp
    ${CMAKE_CURRENT_LIST_DIR}/VectorTest.cpp
    ${CMAKE_CURRENT_LIST_DIR}/StateTest.cpp
    ${CMAKE_CURRENT_LIST_DIR}/BlobTest.cpp
    ${CMAKE_CURRENT_LIST_DIR}/BlobStateTest.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ParticleTest.cpp
)
add_test(NAME Direction COMMAND UnitTests --gtest_filter=Direction*)
add_test(NAME Vector COMMAND UnitTests --gtest_filter=Vector*)
add_test(NAME State COMMAND UnitTests --gtest_filter=State*:-BlobState*)
add_test(NAME Blob COMMAND UnitTests --gtest_filter=Blob*:-BlobState*)
add_test(NAME BlobState COMMAND UnitTests --gtest_filter=BlobState*)
add_test(NAME Particle COMMAND UnitTests --gtest_filter=Particle*)
