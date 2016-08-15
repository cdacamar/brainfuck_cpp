cmake_minimum_required(VERSION 2.8)

project(brainfuck_cpp)

include_directories(./)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
  set(DEBUG_FLAGS, "/GS /W3 /Gm /Zl /Od /Oy-")
  set(RELEASE_FLAGS, "/GS /GL /W3 /Gy /Zi /O2")
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
else()
  set(DEBUG_FLAGS, "-g -O0 -Wall")
  set(RELEASE_FLAGS, "-O3")
endif()

set(CMAKE_CXX_FLAGS_DEBUG ${DEBUG_FLAGS})
set(CMAKE_CXX_FLAGS_RELEASE ${RELEASE_FLAGS})

set(CMAKE_CONFIGURATION_TYPES Debug Release)

file(GLOB_RECURSE CMD_SRC RELATIVE_PATH
      lex/*.cpp
      lex/*.h)
source_group("cmd" FILES ${CMD_SRC})

file(GLOB_RECURSE CODE_GEN_SRC RELATIVE_PATH
      code_gen/*.cpp
      code_gen/*.h)
source_group("code_gen" FILES ${CODE_GEN_SRC})

file(GLOB_RECURSE LEX_SRC RELATIVE_PATH
      lex/*.cpp
      lex/*.h)
source_group("lex" FILES ${LEX_SRC})

file(GLOB_RECURSE OPTIMIZE_SRC RELATIVE_PATH
      optimize/*.cpp
      optimize/*.h)
source_group("optimize" FILES ${OPTIMIZE_SRC})

file(GLOB_RECURSE PARSE_SRC RELATIVE_PATH
      parse/*.cpp
      parse/*.h)
source_group("parse" FILES ${PARSE_SRC})

set(COMPILER_SRC main.cpp
                  ${CMD_SRC}
                  ${CODE_GEN_SRC}
                  ${LEX_SRC}
                  ${OPTIMIZE_SRC}
                  ${PARSE_SRC})

add_executable(brainfuck_cpp ${COMPILER_SRC})

set_property(TARGET brainfuck_cpp PROPERTY FOLDER "compiler")