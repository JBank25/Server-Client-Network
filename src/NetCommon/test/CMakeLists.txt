find_package(GTest REQUIRED)

add_executable(${MODULE_TEST_TARGET} net_ts_queue_tests.cpp)
target_link_libraries(${MODULE_TEST_TARGET} PRIVATE 
                        ${MODULE_TARGET}
                        asio::asio
                        GTest::gtest_main)

add_test(${MODULE_TEST_SUITE} ${MODULE_TEST_TARGET})
