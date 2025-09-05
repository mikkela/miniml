# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/Users/mikkelarentoft/sandbox/github/mikkela/miniml/cmake-build-debug/_deps/antlr4_runtime-src")
  file(MAKE_DIRECTORY "/Users/mikkelarentoft/sandbox/github/mikkela/miniml/cmake-build-debug/_deps/antlr4_runtime-src")
endif()
file(MAKE_DIRECTORY
  "/Users/mikkelarentoft/sandbox/github/mikkela/miniml/cmake-build-debug/_deps/antlr4_runtime-build"
  "/Users/mikkelarentoft/sandbox/github/mikkela/miniml/cmake-build-debug/_deps/antlr4_runtime-subbuild/antlr4_runtime-populate-prefix"
  "/Users/mikkelarentoft/sandbox/github/mikkela/miniml/cmake-build-debug/_deps/antlr4_runtime-subbuild/antlr4_runtime-populate-prefix/tmp"
  "/Users/mikkelarentoft/sandbox/github/mikkela/miniml/cmake-build-debug/_deps/antlr4_runtime-subbuild/antlr4_runtime-populate-prefix/src/antlr4_runtime-populate-stamp"
  "/Users/mikkelarentoft/sandbox/github/mikkela/miniml/cmake-build-debug/_deps/antlr4_runtime-subbuild/antlr4_runtime-populate-prefix/src"
  "/Users/mikkelarentoft/sandbox/github/mikkela/miniml/cmake-build-debug/_deps/antlr4_runtime-subbuild/antlr4_runtime-populate-prefix/src/antlr4_runtime-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/mikkelarentoft/sandbox/github/mikkela/miniml/cmake-build-debug/_deps/antlr4_runtime-subbuild/antlr4_runtime-populate-prefix/src/antlr4_runtime-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/mikkelarentoft/sandbox/github/mikkela/miniml/cmake-build-debug/_deps/antlr4_runtime-subbuild/antlr4_runtime-populate-prefix/src/antlr4_runtime-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
