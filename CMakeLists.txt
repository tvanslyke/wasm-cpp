cmake_minimum_required(VERSION 3.8)

project(wasm-cpp)
FIND_PACKAGE(PythonLibs)
include_directories(./include ${PYTHON_INCLUDE_PATH})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_C_COMPILER gcc)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra")

add_executable(wasm-cpp src/main.cpp src/frontend/create_program.cpp src/interpreter/WasmCallStack.c src/function/wasm_function_storage.c src/function/wasm_function.cpp src/parse/wasm_binary_parser.cpp src/parse/wasm_program_def.cpp src/module/wasm_program_state.cpp)
