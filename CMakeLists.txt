# Infomation
cmake_minimum_required(VERSION 3.5)
project(FluidisedBed LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Glob files
file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
file(GLOB_RECURSE HEADER_FILES include/*.h)
# Glob imgui submodule
set(IMGUI_PATH depend/imgui)
file(GLOB IMGUI_GLOB
    ${IMGUI_PATH}/*.cpp
    ${IMGUI_PATH}/*.h
    # Specific bindings
    ${IMGUI_PATH}/backends/imgui_impl_sdl2.cpp
	${IMGUI_PATH}/backends/imgui_impl_opengl3.cpp
)
INCLUDE_DIRECTORIES(
    ${IMGUI_PATH}
	"/home/user/libs/stb"
)
# Imgui Lib
add_library(imgui STATIC ${IMGUI_GLOB})
target_link_libraries(imgui SDL2)
target_include_directories(imgui PUBLIC SDL2 SDL2main)
# FluidisedBed Lib
add_library(FluidisedBed SHARED
${SOURCE_FILES}
${HEADER_FILES}
)
# Main Simulator executable
add_executable(Simulator src/main.cpp)
target_include_directories( FluidisedBed PUBLIC imgui )
target_link_libraries(Simulator PUBLIC SDL2 FluidisedBed imgui OpenGL)
