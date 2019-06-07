if (COVERAGE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif ()

add_executable(Test 
        ./acceptance_tests.cpp
        ./all_tests.cpp
        ./ccs_test.cpp
        ./context_test.cpp
        ./parser/parser_test.cpp)
target_link_libraries(Test ccs gtest)
add_test(NAME Tests
        COMMAND Test
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ../tests.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(unittest Test
        COMMENT "Running unittests\n\n"
        VERBATIM
        )
