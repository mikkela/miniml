cmake_minimum_required(VERSION 3.22)
project(miniml LANGUAGES CXX)

# Options
option(ENABLE_ANTLR "Enable ANTLR4-based parser" ON)
option(ENABLE_LLVM  "Enable LLVM backend" OFF)  # turn ON if LLVM is installed
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Warnings
if (MSVC)
  add_compile_options(/W4 /permissive-)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter)
endif()

# GoogleTest (FetchContent)
include(FetchContent)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
FetchContent_MakeAvailable(googletest)

# ANTLR4 runtime (optional)
if (ENABLE_ANTLR)
  FetchContent_Declare(
    antlr4_runtime
    GIT_REPOSITORY https://github.com/antlr/antlr4.git
    GIT_TAG        4.13.1
  )
  set(ANTLR4_WITH_STATIC_CRT OFF CACHE BOOL "" FORCE)
  set(ANTLR_BUILD_CPP_TESTS OFF CACHE BOOL "" FORCE)
  set(ANTLR_BUILD_RUNTIME    ON  CACHE BOOL "" FORCE)
  set(ANTLR_BUILD_TOOLS      OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(antlr4_runtime)
  add_compile_definitions(MINIML_ENABLE_ANTLR=1)
endif()

# LLVM (optional)
if (ENABLE_LLVM)
  find_package(LLVM REQUIRED CONFIG)
  message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
  message(STATUS "LLVM Include dirs: ${LLVM_INCLUDE_DIRS}")
  message(STATUS "LLVM Definitions: ${LLVM_DEFINITIONS}")
  include_directories(${LLVM_INCLUDE_DIRS})
  add_compile_definitions(MINIML_ENABLE_LLVM=1)
endif()

# Sources
file(GLOB_RECURSE MINIML_SRC
  "src/*.cpp"
  "src/*.hpp"
  "src/*.h"
)

add_library(miniml STATIC ${MINIML_SRC})
target_include_directories(miniml PUBLIC src)

if (ENABLE_ANTLR)
  target_link_libraries(miniml PUBLIC antlr4_shared)
endif()

if (ENABLE_LLVM)
  llvm_map_components_to_libnames(LLVM_LIBS core orcjit native)
  target_link_libraries(miniml PUBLIC ${LLVM_LIBS})
endif()

# CLI
add_executable(minimlc src/main.cpp)
target_link_libraries(minimlc PRIVATE miniml)

# Tests
enable_testing()
add_executable(miniml_tests tests/test_typechecker.cpp)
target_link_libraries(miniml_tests PRIVATE miniml gtest_main)
include(GoogleTest)
gtest_discover_tests(miniml_tests)
