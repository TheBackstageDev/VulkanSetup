#pragma once

#include <volk/volk.h>
#include "vk_context.hpp"

#include <optional>

namespace vk
{
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    class vk_device
    {
    public:
        vk_device(vk_context& context);
        ~vk_device();

        const VkDevice& device() { return _device; }
        const VkPhysicalDevice phydevice() { return _physical_device; }

        VkQueue graphicsQueue() const { return _graphicsQueue; }
        VkQueue presentQueue() const { return _presentQueue; }

        uint32_t graphicsFamily() const { return _queueFamilies.graphicsFamily.value(); }
        uint32_t presentFamily() const { return _queueFamilies.presentFamily.value(); }
    private:

        void pickPhydevice(vk_context& context);
        void createDevice(vk_context& context);
        void createAllocator(vk_context& context);

        bool isDeviceSuitable(VkPhysicalDevice& device, vk_context& context);

        QueueFamilyIndices findQueueFamilies(vk_context& context);

        VkDevice _device = VK_NULL_HANDLE;
        VkPhysicalDevice _physical_device = VK_NULL_HANDLE;

        vk_context& context;

        VkQueue _graphicsQueue;
        VkQueue _presentQueue;

        QueueFamilyIndices _queueFamilies;

        std::vector<const char*> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    };
} // namespace vk
