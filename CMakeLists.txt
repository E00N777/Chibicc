cmake_minimum_required(VERSION 3.10)

project(Chibicc VERSION 0.1.0 LANGUAGES CXX)



set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(cc_core STATIC
    lib/tokenize.cpp
)

target_include_directories(cc_core PUBLIC "${CMAKE_SOURCE_DIR}/include")


add_executable(Chibicc
    Main.cpp
)

target_link_libraries(Chibicc PRIVATE cc_core)
enable_testing()

add_test(NAME IntegrationTest 
         COMMAND ${CMAKE_SOURCE_DIR}/test.sh $<TARGET_FILE:Chibicc>)
