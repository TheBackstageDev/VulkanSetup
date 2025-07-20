cmake_minimum_required(VERSION 3.20)

project(vkengine VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(include)

find_package(Vulkan REQUIRED)
file(GLOB_RECURSE SRC_FILES src/*.cpp src/*.hpp)

add_executable(${PROJECT_NAME} ${SRC_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC
    src/
    include/
    include/vma/
    include/volk/
    include/vulkan/
    include/glfw/include
    include/glm/
    include/stb/
    "$ENV{VULKAN_SDK}/Include"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    volk::volk
    glfw
    imgui
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE VK_NO_PROTOTYPES)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Running shader compilation script..."
    COMMAND ${CMAKE_SOURCE_DIR}/src/shaders/compiler.bat
    COMMENT "Compiling shaders..."
)
