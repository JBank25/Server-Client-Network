find_package(GTest REQUIRED)

add_executable(ModuleTests module_test.cpp)
target_link_libraries(ModuleTests PRIVATE GTest::gtest_main)
add_test(ModuleGTests ModuleTests)
