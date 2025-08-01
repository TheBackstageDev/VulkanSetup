cmake_minimum_required(VERSION 3.20)

project(vkdeps)

set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(glm)
add_subdirectory(glfw)
add_subdirectory(volk)
add_subdirectory(assimp)