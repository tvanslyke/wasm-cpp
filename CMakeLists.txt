cmake_minimum_required(VERSION 3.8)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()
message("${CMAKE_BUILD_TYPE}")
project(wasm-cpp)
FIND_PACKAGE(PythonLibs)
set(PYTHON_DEBUG_INCLUDE_DIRS "/usr/include/python3.6dm")

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
	include_directories(./include ${PYTHON_DEBUG_INCLUDE_DIRS})
else()
	include_directories(./include ${PYTHON_INCLUDE_DIRS})
endif("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

set(CMAKE_CXX_COMPILER g++)
set(CMAKE_C_COMPILER gcc)



set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -std=c11 -Wall -g -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -std=c++17 -g -Wall -Wextra")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -std=c11 -Wall -O3 -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -std=c++17 -O3 -Wall -Wextra")

set(PYTHON_DEBUG_LIBRARIES "/usr/lib/x86_64-linux-gnu/libpython3.6dm.so")

add_executable(wasm-cpp src/main.cpp src/frontend/create_program.cpp src/frontend/leb128/leb128.c src/module/wasm_program_state.cpp)

target_link_libraries(wasm-cpp optimized ${PYTHON_LIBRARIES} debug ${PYTHON_DEBUG_LIBRARIES})


