function(add_flow_test TEST_NAME)
    add_executable(${TEST_NAME} 
        test/${TEST_NAME}.cpp 
        flow/src/types/value.cpp 
        flow/src/types/function.cpp)
    
    target_include_directories(${TEST_NAME} PUBLIC flow/src)
endfunction()

add_flow_test(syntax_test)

