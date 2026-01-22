cmake_minimum_required(VERSION 3.10)

project(Chibicc VERSION 0.1.0 LANGUAGES CXX)



set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_executable(Chibicc
    Main.cpp
)

enable_testing()

add_test(NAME IntegrationTest 
         COMMAND ${CMAKE_SOURCE_DIR}/test.sh $<TARGET_FILE:Chibicc>)
