cmake_minimum_required(VERSION 3.10)

project(imgui)

file(GLOB IMGUI_FILES imgui/*.cpp imgui/*.h)

message(${CMAKE_CURRENT_SOURCE_DIR})

set(IMGUI_BACKEND_FILES ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.cpp ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_glfw.h ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.cpp ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends/imgui_impl_opengl3.h)

add_library(imgui ${IMGUI_FILES} ${IMGUI_BACKEND_FILES})

include_directories(imgui
  ../glfw/include/
)

target_include_directories(imgui
  PUBLIC imgui
)