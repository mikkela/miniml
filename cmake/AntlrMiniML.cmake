# AntlrMiniML.cmake
#
# Usage in top-level CMakeLists.txt (AFTER add_library(miniml ...)):
#
#   list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#   include(AntlrMiniML)
#
#   miniml_add_antlr_codegen(
#     TARGET        miniml
#     GRAMMAR       ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer_parser/MiniML.g4
#     ENABLE_GEN    ON              # or OFF if you pre-generate sources
#     ANTLR_VERSION 4.13.1
#   )

function(miniml_add_antlr_codegen)
  cmake_parse_arguments(MA "" "TARGET;GRAMMAR;ENABLE_GEN;ANTLR_VERSION" "" ${ARGN})

  if (NOT MA_TARGET)
    message(FATAL_ERROR "miniml_add_antlr_codegen: missing TARGET")
  endif()
  if (NOT TARGET ${MA_TARGET})
    message(FATAL_ERROR "miniml_add_antlr_codegen: '${MA_TARGET}' does not exist; call this AFTER add_library/add_executable.")
  endif()
  if (NOT MA_GRAMMAR)
    message(FATAL_ERROR "miniml_add_antlr_codegen: missing GRAMMAR")
  endif()
  if (NOT MA_ANTLR_VERSION)
    set(MA_ANTLR_VERSION 4.13.1)
  endif()

  # Directories
  get_filename_component(_GRAMMAR_DIR ${MA_GRAMMAR} DIRECTORY)
  set(_GEN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/lexer_parser/gen)
  file(MAKE_DIRECTORY ${_GEN_DIR})

  # Always include generated headers
  target_include_directories(${MA_TARGET} PRIVATE ${_GEN_DIR})

  # If code generation is disabled, just compile what's in gen/
  if (NOT MA_ENABLE_GEN)
    file(GLOB _GEN_SRC "${_GEN_DIR}/*.cpp" "${_GEN_DIR}/*.h")
    if (_GEN_SRC)
      target_sources(${MA_TARGET} PRIVATE ${_GEN_SRC})
    else()
      message(WARNING "miniml_add_antlr_codegen: ENABLE_GEN=OFF but no files in ${_GEN_DIR}")
    endif()
    return()
  endif()

  # ---- Build-time generation ----
  find_package(Java COMPONENTS Runtime REQUIRED)

  set(_ANTLR_JAR ${CMAKE_BINARY_DIR}/antlr-${MA_ANTLR_VERSION}-complete.jar)
  if (NOT EXISTS ${_ANTLR_JAR})
    message(STATUS "Downloading ANTLR ${MA_ANTLR_VERSION} tool jar...")
    file(DOWNLOAD
            https://www.antlr.org/download/antlr-${MA_ANTLR_VERSION}-complete.jar
            ${_ANTLR_JAR}
            SHOW_PROGRESS
    )
  endif()

  set(_GEN_SOURCES
          ${_GEN_DIR}/MiniMLLexer.cpp
          ${_GEN_DIR}/MiniMLLexer.h
          ${_GEN_DIR}/MiniMLParser.cpp
          ${_GEN_DIR}/MiniMLParser.h
          ${_GEN_DIR}/MiniMLBaseVisitor.cpp
          ${_GEN_DIR}/MiniMLBaseVisitor.h
          ${_GEN_DIR}/MiniMLVisitor.cpp
          ${_GEN_DIR}/MiniMLVisitor.h
  )

  # Run ANTLR in the grammar directory and pass a relative path to prevent nested gen/src/lexer_parser duplicates.
  add_custom_command(
          OUTPUT ${_GEN_SOURCES}
          WORKING_DIRECTORY ${_GRAMMAR_DIR}
          COMMAND ${Java_JAVA_EXECUTABLE} -jar ${_ANTLR_JAR}
          -Dlanguage=Cpp -visitor -no-listener
          -o ${_GEN_DIR} MiniML.g4
          DEPENDS ${MA_GRAMMAR}
          COMMENT "Generating C++ lexer/parser from MiniML.g4"
          VERBATIM
  )

  add_custom_target(${MA_TARGET}_antlr_gen DEPENDS ${_GEN_SOURCES})
  add_dependencies(${MA_TARGET} ${MA_TARGET}_antlr_gen)
  target_sources(${MA_TARGET} PRIVATE ${_GEN_SOURCES})
endfunction()
