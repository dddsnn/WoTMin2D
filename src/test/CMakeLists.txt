add_executable(UnitTests "")
target_compile_options(UnitTests PUBLIC ${COMPILE_OPTIONS})
target_link_libraries(UnitTests Game)

add_test(NAME UnitTests COMMAND UnitTests)
target_sources(UnitTests PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)
