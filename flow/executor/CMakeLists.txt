add_executable(flow
    flow/executor/flow.cpp 
    flow/src/types/value.cpp 
    flow/src/types/function.cpp)

target_include_directories(flow PUBLIC flow/src)

