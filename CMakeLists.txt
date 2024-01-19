cmake_minimum_required(VERSION 3.27)

project(minesweeper CXX)

set(CMAKE_CXX_STANDARD 17)
set(SOURCES main.cpp)

set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc")
add_executable(minesweeper WIN32 ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

# raylib library
include(FetchContent)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games
FetchContent_Declare(raylib GIT_REPOSITORY https://github.com/raysan5/raylib.git GIT_TAG master)
FetchContent_MakeAvailable(raylib)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:-DDEBUG>)
