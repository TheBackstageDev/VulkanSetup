#pragma once

#include <volk/volk.h>
#include <GLFW/glfw3.h>

#include <vector>

namespace vk
{   
    class vk_window
    {
    public:
        vk_window(int32_t width, int32_t height, const char* title);
        ~vk_window();

        vk_window(const vk_window &) = delete;
        vk_window &operator=(const vk_window &) = delete;

        const char* get_title() { return title; }

        GLFWwindow* window() { return p_window; }

        std::pair<int32_t, int32_t> dimensions() const 
        {
            int32_t width, height;

            glfwGetWindowSize(p_window, &width, &height);

            return std::pair<int32_t, int32_t>(width, height); 
        }

        bool should_close() { return glfwWindowShouldClose(p_window); }

        operator GLFWwindow *() const { return p_window; }
    private:
        const char* title = nullptr;
        GLFWwindow* p_window = nullptr;
    };
} // namespace vk
