cmake_minimum_required(VERSION 3.23)
project(asio_sandbox)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

message(STATUS "Buidling with CMake version: ${CMAKE_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Testing ENABLE_UNIT_TESTING: ${ENABLE_UNIT_TESTING}")

find_package(asio REQUIRED)

enable_testing()

add_executable(${PROJECT_NAME} src/main.cpp)

add_subdirectory(src/)

target_link_libraries(${PROJECT_NAME} PUBLIC 
                        asio::asio
                        NetCommon
                        NetServer
                        NetConnection
                        )

target_include_directories(${PROJECT_NAME} PUBLIC
                    "${PROJECT_BINARY_DIR}"
                    "${PROJECT_SOURCE_DIR}/src/NetCommon"
                    "${PROJECT_SOURCE_DIR}/src/NetServer"
                    "${PROJECT_SOURCE_DIR}/src/NetConnection"
)