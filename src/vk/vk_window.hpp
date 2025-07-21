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

        VkExtent2D extent() const 
        {
            int32_t width, height;

            glfwGetWindowSize(p_window, &width, &height);

            return VkExtent2D{static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; 
        }

        bool should_close() { return glfwWindowShouldClose(p_window); }
        bool resized() { return _resized; }

        void resetResizedFlag() { _resized = false; }

        operator GLFWwindow *() const { return p_window; }
    private:
        static void frameBufferResizeCallback(GLFWwindow *window, int width, int height);

        bool _resized = false;
        
        const char* title = nullptr;
        GLFWwindow* p_window = nullptr;
    };
} // namespace vk
