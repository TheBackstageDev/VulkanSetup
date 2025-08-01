#include "vk_window.hpp"
#include <iostream>

namespace vk
{
    vk_window::vk_window(int32_t width, int32_t height, const char* title)
    : title(title)
    {
        if (!glfwInit())
        {
            std::cerr << "Failed to initialize GLFW" << std::endl;
            exit(EXIT_FAILURE);
        }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        p_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
        if (!p_window)
        {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            exit(EXIT_FAILURE);
        }

        glfwSetWindowUserPointer(p_window, this);
        glfwSetFramebufferSizeCallback(p_window, frameBufferResizeCallback);
    }

    vk_window::~vk_window()
    {
        glfwDestroyWindow(p_window);
        glfwTerminate();
    }

    void vk_window::frameBufferResizeCallback(GLFWwindow *pWindow, int width, int height)
    {
        auto window = reinterpret_cast<vk_window *>(glfwGetWindowUserPointer(pWindow));
        window->_resized = true;
    }
} // namespace vk
