add_executable(Test 
        ./acceptance_tests.cpp
        ./all_tests.cpp
        ./ccs_test.cpp
        ./context_test.cpp
        ./parser/parser_test.cpp)
target_link_libraries(Test libccs gtest)
add_test(NAME Tests
        COMMAND Test
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ../tests.txt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
