cmake_minimum_required(VERSION 3.10.0)
project(LDustSim VERSION 0.1.0 LANGUAGES C CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

set(CMAKE_CUDA_ARCHITECTURES 75 80 89)

include(FetchContent)

FetchContent_Declare(
    raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/6.0.tar.gz
)
FetchContent_MakeAvailable(raylib)

FetchContent_Declare(
    luabridge3
    GIT_REPOSITORY https://github.com/kunitoki/LuaBridge3.git
    GIT_TAG        master
)

set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)


add_executable(${PROJECT_NAME} 
    src/Main.cpp
    src/ParticleSimulator.cpp
    src/Kernel.cu
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    raylib
)

target_include_directories(${PROJECT_NAME} PRIVATE 
    src
    ${luabridge3_SOURCE_DIR}/Source
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(${PROJECT_NAME} PRIVATE m pthread dl rt X11)
endif()
