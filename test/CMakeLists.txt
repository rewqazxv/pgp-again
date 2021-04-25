cmake_minimum_required(VERSION 3.5)

project(unittest LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest REQUIRED)

add_executable(${PROJECT_NAME}
    test-rsa.cpp
    test-aes.cpp
    ../src/encrypt/aes.c
    ../src/encrypt/rsa.cpp
)

target_link_libraries(${PROJECT_NAME}
    gtest
    gtest_main
    gmp
    gmpxx
)
