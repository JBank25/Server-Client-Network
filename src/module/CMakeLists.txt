add_library(Module 
    src/module.cpp)

target_include_directories(Module 
    PUBLIC 
    include/)
add_subdirectory(test)
