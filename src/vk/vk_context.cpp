#include "vk_context.hpp"

#include <iostream>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace vk
{
    VmaAllocator vk_context::allocator = nullptr;
    VkDevice vk_context::device = VK_NULL_HANDLE;
    VkCommandPool vk_context::commandPool = VK_NULL_HANDLE;

    VkQueue vk_context::graphicsQueue = VK_NULL_HANDLE;
    VkQueue vk_context::presentQueue = VK_NULL_HANDLE;    

    VkFormat vk_context::imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
    VkFormat vk_context::depthFormat = VK_FORMAT_D32_SFLOAT;

    void vk_context::init(const vkContextCreateInfo info)
    {
        createInstance(info);

        #ifdef VK_VALIDATION_LAYERS
            createDebugTools();
        #endif

        createSurface(info.pWindow);

        extensions = info.instance_extensions;
        layers = info.instance_layers;
    }

    void vk_context::shutdown()
    {
        #ifdef VK_VALIDATION_LAYERS
            destroyDebugTools();
        #endif

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vmaDestroyAllocator(allocator);

        vkDestroyDevice(device, nullptr);
        vkDestroyInstance(instance, nullptr);
    }

    // Initializers

    void vk_context::createInstance(const vkContextCreateInfo& info)
    {
        VkApplicationInfo appInfo{};
        appInfo.apiVersion = VK_API_VERSION_1_3;
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pApplicationName = info.pWindow->get_title();
        appInfo.pEngineName = "vksetup";
        
        VkInstanceCreateInfo createInfo{};
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        std::vector<const char*> finalExtensions = info.instance_extensions;

        for (uint32_t i = 0; i < glfwExtensionCount; ++i)
            finalExtensions.push_back(glfwExtensions[i]);

        if (!extensionsAvailable(finalExtensions) || !layersAvailable(info.instance_layers))
        {
            throw std::runtime_error("Required Vulkan extensions or layers are unavailable!");
        }

        createInfo.ppEnabledLayerNames = info.instance_layers.data();
        createInfo.ppEnabledExtensionNames = finalExtensions.data();

        createInfo.enabledExtensionCount = static_cast<uint32_t>(finalExtensions.size());
        createInfo.enabledLayerCount = static_cast<uint32_t>(info.instance_layers.size());

        vkCreateInstance(&createInfo, nullptr, &instance);

        volkLoadInstance(instance);
    }

    void vk_context::createSurface(vk_window* pWindow)
    {
        if (glfwCreateWindowSurface(instance, pWindow->window(), nullptr, &surface) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create the window surface!");
        }
    }

    // checking

    bool vk_context::extensionsAvailable(const std::vector<const char*>& extensions)
    {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());
        
        for (const char* required : extensions)
        {
            bool found = false;

            for (const auto& ext : availableExtensions)
            {
                if (strcmp(required, ext.extensionName))
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                std::cerr << "Vulkan extension: " << required << " Not available!" << std::endl;
                return false;
            }
        }

        return true;
    }

    bool vk_context::layersAvailable(const std::vector<const char*>& layers)
    {
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* required : layers)
        {
            bool found = false;
            for (const auto& layer : availableLayers)
            {
                if (strcmp(required, layer.layerName) == 0)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                std::cerr << "Vulkan layer: " << required << " Not available!" << std::endl;
                return false;
            }
        }
        return true;
    }

    // Debug Initializing

    VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT severity,
        VkDebugUtilsMessageTypeFlagsEXT type,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void* userData) 
    {
        std::cerr << "Validation Layer: " << callbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    void vk_context::createDebugTools()
    {
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

        createInfo.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        createInfo.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        createInfo.pUserData = nullptr;
        createInfo.pfnUserCallback = debugCallback;

        auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

        if (func && func(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("Failed to set up debug messenger!");
        }
    }

    void vk_context::destroyDebugTools()
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance,
            "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debugMessenger, nullptr);
        }
    }

    // others

    VkCommandBuffer vk_context::beginSingleTimeCommand()
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer cmd;
        if (vkAllocateCommandBuffers(device, &allocInfo, &cmd) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate command buffer for single time commands");

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("Failed to begin single time command buffer");

        return cmd;
    }

    void vk_context::endSingleTimeCommand(VkCommandBuffer cmd)
    {
        if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
            throw std::runtime_error("Failed to end single time command buffer");

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &cmd;

        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
            throw std::runtime_error("Failed to submit single time command buffer");

        vkQueueWaitIdle(graphicsQueue);

        vkFreeCommandBuffers(device, commandPool, 1, &cmd);
    }
} // namespace vk
