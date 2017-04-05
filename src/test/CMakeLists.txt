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
    ${CMAKE_CURRENT_LIST_DIR}/BlobTest.cpp
    # TODO Add back         
    #    ${CMAKE_CURRENT_LIST_DIR}/ParticlePositionStateTest.cpp
    #    ${CMAKE_CURRENT_LIST_DIR}/ParticlePressureStateTest.cpp
)
add_test(NAME Direction COMMAND UnitTests --gtest_filter=Direction*)
add_test(NAME Vector COMMAND UnitTests --gtest_filter=Vector*)
add_test(NAME Blob COMMAND UnitTests --gtest_filter=Blob*)
# TODO Add back         
#add_test(NAME ParticlePositionState
#         COMMAND UnitTests --gtest_filter=ParticlePositionState*)
#add_test(NAME ParticlePressureState
#         COMMAND UnitTests --gtest_filter=ParticlePressureState*)
