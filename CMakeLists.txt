cmake_minimum_required(VERSION 3.22)
project(miniml LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----------------------------
# Options
# ----------------------------
option(ENABLE_GTEST      "Fetch and enable GoogleTest targets" ON)
option(ENABLE_ANTLR      "Enable ANTLR4 (runtime + codegen)"   ON)
option(ENABLE_ANTLR_GEN  "Generate parser at build time (needs Java)" ON)

# ----------------------------
# Dependencies via FetchContent
# ----------------------------
include(FetchContent)

if (ENABLE_GTEST)
  set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(
          googletest
          GIT_REPOSITORY https://github.com/google/googletest.git
          GIT_TAG        v1.14.0
  )
  FetchContent_MakeAvailable(googletest)
endif()

# ----------------------------
# Core target
# ----------------------------
file(GLOB_RECURSE MINIML_SRC
        "src/*.cpp"
        "src/*.hpp"
        "src/*.h"
)
if (MINIML_SRC STREQUAL "")
  message(WARNING "No sources found under src/. Ensure you have at least one .cpp file.")
endif()

add_library(miniml STATIC ${MINIML_SRC})
target_include_directories(miniml PUBLIC src)

# ----------------------------
# ANTLR runtime: try system package first, fallback to FetchContent
# ----------------------------
set(MINIML_HAVE_ANTLR OFF)
set(MINIML_ANTLR_TARGET "")

if (ENABLE_ANTLR)
  # 1) Try find_package (works with Homebrew: `brew install antlr4-cpp-runtime`)
  #    Homebrew typically installs a CMake config package "antlr4-runtime".
  find_package(antlr4-runtime QUIET CONFIG)

  # Try a handful of possible target names exported by different packages
  foreach(cand antlr4_shared antlr4_static antlr4-runtime::antlr4-runtime antlr4::antlr4)
    if (TARGET ${cand})
      set(MINIML_ANTLR_TARGET ${cand})
      set(MINIML_HAVE_ANTLR ON)
      break()
    endif()
  endforeach()

  # 2) Fallback: FetchContent the official repo and build the C++ runtime
  if (NOT MINIML_HAVE_ANTLR)
    message(STATUS "antlr4-runtime not found via find_package; falling back to FetchContent.")
    FetchContent_Declare(
            antlr4_runtime_src
            GIT_REPOSITORY https://github.com/antlr/antlr4.git
            GIT_TAG        4.13.1
    )
    # Build only the C++ runtime; not the tools, not the tests
    set(ANTLR_BUILD_CPP_TESTS OFF CACHE BOOL "" FORCE)
    set(ANTLR_BUILD_RUNTIME    ON  CACHE BOOL "" FORCE)
    set(ANTLR_BUILD_TOOLS      OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(antlr4_runtime_src)

    # The runtime subproject usually defines these targets:
    if (TARGET antlr4_shared)
      set(MINIML_ANTLR_TARGET antlr4_shared)
      set(MINIML_HAVE_ANTLR ON)
    elseif (TARGET antlr4_static)
      set(MINIML_ANTLR_TARGET antlr4_static)
      set(MINIML_HAVE_ANTLR ON)
    endif()

    # Fallback include dir if target doesn't export includes
    if (DEFINED antlr4_runtime_src_SOURCE_DIR)
      target_include_directories(miniml PRIVATE
              ${antlr4_runtime_src_SOURCE_DIR}/runtime/Cpp/runtime/src
      )
    endif()
  endif()

  if (MINIML_HAVE_ANTLR)
    target_link_libraries(miniml PRIVATE ${MINIML_ANTLR_TARGET})
    # Propagate includes to dependents (CLIs/tests) ➜ PUBLIC
    get_target_property(_ANTLR_INC ${MINIML_ANTLR_TARGET} INTERFACE_INCLUDE_DIRECTORIES)
    if (_ANTLR_INC)
      target_include_directories(miniml PUBLIC ${_ANTLR_INC})
    endif()

    # Fallback include dir (also PUBLIC so it propagates)
    if (DEFINED antlr4_runtime_src_SOURCE_DIR)
      target_include_directories(miniml PUBLIC
              ${antlr4_runtime_src_SOURCE_DIR}/runtime/Cpp/runtime/src
      )
    endif()
  else()
    message(WARNING "ANTLR runtime could not be found or built. You can:\n"
            "  - install via Homebrew: brew install antlr4-cpp-runtime\n"
            "  - or let FetchContent build it (check network/SSL/proxy)\n"
            "  - or set ENABLE_ANTLR=OFF for now")
  endif()
endif() # ENABLE_ANTLR

# ----------------------------
# ANTLR codegen (JAR) and generated sources
# ----------------------------
if (ENABLE_ANTLR)
  set(GRAMMAR_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer_parser)
  set(GRAMMAR_FILE ${GRAMMAR_DIR}/MiniML.g4)
  set(GEN_DIR      ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer_parser/gen)
  file(MAKE_DIRECTORY ${GEN_DIR})

  # Always include generated headers
  target_include_directories(miniml PUBLIC ${GEN_DIR})

  if (ENABLE_ANTLR_GEN)
    # Build-time generation using Java + tool jar
    find_package(Java COMPONENTS Runtime QUIET)
    if (NOT Java_JAVA_EXECUTABLE)
      message(FATAL_ERROR "Java not found but ENABLE_ANTLR_GEN=ON. Install Java or set ENABLE_ANTLR_GEN=OFF.")
    endif()

    set(ANTLR_VERSION 4.13.1)
    set(ANTLR_JAR ${CMAKE_BINARY_DIR}/antlr-${ANTLR_VERSION}-complete.jar)
    if (NOT EXISTS ${ANTLR_JAR})
      message(STATUS "Downloading ANTLR ${ANTLR_VERSION} tool jar...")
      file(DOWNLOAD
              https://www.antlr.org/download/antlr-${ANTLR_VERSION}-complete.jar
              ${ANTLR_JAR} SHOW_PROGRESS
      )
    endif()

    set(GEN_SOURCES
            ${GEN_DIR}/MiniMLLexer.cpp
            ${GEN_DIR}/MiniMLLexer.h
            ${GEN_DIR}/MiniMLParser.cpp
            ${GEN_DIR}/MiniMLParser.h
            ${GEN_DIR}/MiniMLBaseVisitor.cpp
            ${GEN_DIR}/MiniMLBaseVisitor.h
            ${GEN_DIR}/MiniMLVisitor.cpp
            ${GEN_DIR}/MiniMLVisitor.h
    )

    add_custom_command(
            OUTPUT ${GEN_SOURCES}
            WORKING_DIRECTORY ${GRAMMAR_DIR}         # prevents nested gen/src/lexer_parser
            COMMAND ${Java_JAVA_EXECUTABLE} -jar ${ANTLR_JAR}
            -Dlanguage=Cpp -visitor -no-listener
            -o ${GEN_DIR} MiniML.g4
            DEPENDS ${GRAMMAR_FILE}
            COMMENT "Generating C++ lexer/parser from MiniML.g4"
            VERBATIM
    )

    add_custom_target(antlr_gen DEPENDS ${GEN_SOURCES})
    add_dependencies(miniml antlr_gen)
    target_sources(miniml PRIVATE ${GEN_SOURCES})
  else()
    # Pre-generated path: compile whatever is already in gen/
    file(GLOB MINIML_GEN_SRC "${GEN_DIR}/*.cpp" "${GEN_DIR}/*.h")
    if (MINIML_GEN_SRC)
      target_sources(miniml PRIVATE ${MINIML_GEN_SRC})
    else()
      message(WARNING "ENABLE_ANTLR_GEN=OFF but no generated files found in ${GEN_DIR}.\n"
              "Run the tool once manually:\n"
              "  java -jar antlr-4.13.1-complete.jar -Dlanguage=Cpp -visitor -no-listener \\\n"
              "    -o src/lexer_parser/gen src/lexer_parser/MiniML.g4")
    endif()
  endif()
endif()

# ----------------------------
# CLI tools (optional)
# ----------------------------
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/parser/parse_cli.cpp)
  add_executable(miniml_parse_cli src/parser/parse_cli.cpp)
  target_link_libraries(miniml_parse_cli PRIVATE miniml)
endif()

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/parser/lex_cli.cpp)
  add_executable(miniml_lex_cli src/parser/lex_cli.cpp)
  target_link_libraries(miniml_lex_cli PRIVATE miniml)
endif()

# ----------------------------
# Tests (optional)
# ----------------------------
if (ENABLE_GTEST AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_parser.cpp)
  enable_testing()
  add_executable(miniml_tests
          tests/test_parser.cpp
  )
  target_link_libraries(miniml_tests PRIVATE miniml gtest_main)
  include(GoogleTest)
  gtest_discover_tests(miniml_tests)
endif()
