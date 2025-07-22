#pragma once

#include "vk_window.hpp"
#include <vector>
#include <volk.h>

#include <vma/vk_mem_alloc.h>

#define VK_VALIDATION_LAYERS

namespace vk
{
    struct vkContextCreateInfo
    {
        vk_window* pWindow = nullptr;

        #ifdef VK_VALIDATION_LAYERS
            std::vector<const char*> instance_layers = {"VK_LAYER_KHRONOS_validation"};
            std::vector<const char*> instance_extensions = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
        #else
            std::vector<const char*> instance_layers;
            std::vector<const char*> instance_extensions;
        #endif
    };

    class vk_context
    {
    public:
        void init(const vkContextCreateInfo info);
        void shutdown();

        const VkInstance vk_instance() { return instance; }
        const VkSurfaceKHR vk_surface() { return surface; }
        const VkDevice vk_device() { return device; }
        static VmaAllocator& vk_allocator() { return allocator; }

        std::vector<const char*> vk_extensions() { return extensions; }
        std::vector<const char*> vk_layers() { return layers; }

        void setDeviceHandle(VkDevice handle) { device = handle; }

    private:
        VkInstance instance;
        VkSurfaceKHR surface;
        VkDevice device;
        static VmaAllocator allocator;
        
        VkDebugUtilsMessengerEXT debugMessenger;

        void createInstance(const vkContextCreateInfo& info);
        void createSurface(vk_window* pWindow);

        // checking

        bool extensionsAvailable(const std::vector<const char*>& extensions);
        bool layersAvailable(const std::vector<const char*>& layers);

        // debug set-up

        bool isDebugAvailable();
        void createDebugTools();
        void destroyDebugTools();

        // data
        std::vector<const char*> layers;
        std::vector<const char*> extensions;
    };
} // namespace vk
