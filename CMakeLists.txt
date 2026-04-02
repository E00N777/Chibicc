cmake_minimum_required(VERSION 3.10)

project(Casting VERSION 0.1.0 LANGUAGES CXX)



set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, defaulting to Debug (Symbols enabled)")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

add_library(casting_core STATIC
    lib/diagnostic.cpp
    lib/tokenize.cpp
    lib/parser.cpp
    lib/codegen.cpp
    lib/type.cpp
)

target_include_directories(casting_core PUBLIC "${CMAKE_SOURCE_DIR}/include")


add_executable(Casting
    Main.cpp
)

target_link_libraries(Casting PRIVATE casting_core)
enable_testing()

add_test(NAME IntegrationTest 
         COMMAND ${CMAKE_SOURCE_DIR}/test.sh $<TARGET_FILE:Casting>)
