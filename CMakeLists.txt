cmake_minimum_required(VERSION 3.20)

project(vkengine VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(include)

find_package(Vulkan REQUIRED)
file(GLOB_RECURSE SRC_FILES src/*.cpp src/*.hpp)

file(GLOB IMGUI_CORE_FILES ${CMAKE_SOURCE_DIR}/include/imgui/*.cpp ${CMAKE_SOURCE_DIR}/include/imgui/*.h)
file(GLOB IMGUI_BACKEND_FILES ${CMAKE_SOURCE_DIR}/include/imgui/backends/imgui_impl_glfw.cpp ${CMAKE_SOURCE_DIR}/include/imgui/backends/imgui_impl_vulkan.cpp)

add_executable(${PROJECT_NAME} ${SRC_FILES} ${IMGUI_CORE_FILES} ${IMGUI_BACKEND_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC
    src/
    include/
    include/vma/
    include/volk/
    include/vulkan/
    include/glfw/include/
    include/glm/
    include/stb/
    include/assimp/
    include/imgui/
    "$ENV{VULKAN_SDK}/Include"
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
if(MSVC)
    set_property(TARGET vkengine PROPERTY VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    volk::volk
    glfw
    assimp
)

target_compile_definitions(${PROJECT_NAME} PRIVATE 
    VK_NO_PROTOTYPES
    IMGUI_IMPL_VULKAN_NO_PROTOTYPES
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            $<TARGET_FILE:${PROJECT_NAME}>
            ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.exe
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/src/resource/icons
            ${CMAKE_BINARY_DIR}/Debug/icons
)