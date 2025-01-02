cmake_minimum_required(VERSION 3.23)
project(asio_sandbox)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(asio REQUIRED)

enable_testing()

add_executable(${PROJECT_NAME} src/main.cpp)

add_subdirectory(src/module)

target_link_libraries(${PROJECT_NAME} PUBLIC 
                        Module
                        asio::asio)

target_include_directories(${PROJECT_NAME} PUBLIC
                    "${PROJECT_BINARY_DIR}"
                    "${PROJECT_SOURCE_DIR}/src/module"
)