cmake_minimum_required(VERSION 3.10)

project(Database)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # Exists so nvim lsp can find headers

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/out)

include_directories(src/util)
aux_source_directory(src/util UTIL_SOURCES)

add_executable(create_files src/create_files.cpp ${UTIL_SOURCES})
add_executable(sort_files src/sort_files.cpp ${UTIL_SOURCES})
