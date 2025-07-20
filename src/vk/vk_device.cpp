#include "vk_device.hpp"
#include <iostream>
#include <set>

namespace vk
{
    vk_device::vk_device(vk_context& context)
        : context(context)
    {
        pickPhydevice(context);
        createDevice(context);
        createAllocator(context);
    }

    vk_device::~vk_device()
    {
        context.shutdown();
        vkDestroyDevice(_device, nullptr);
    }

    void vk_device::pickPhydevice(vk_context& context)
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(context.vk_instance(), &deviceCount, nullptr);

        if (deviceCount == 0)
            throw std::runtime_error("No Vulkan-compatible physical devices found!");

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(context.vk_instance(), &deviceCount, devices.data());

        for (auto& device : devices) {
            if (isDeviceSuitable(device, context)) {
                _physical_device = device;
                break;
            }
        }

        if (_physical_device == VK_NULL_HANDLE)
            throw std::runtime_error("No suitable physical device found!");
    }

    void vk_device::createDevice(vk_context& context)
    {
        _queueFamilies = findQueueFamilies(context);

        float queuePriority = 1.0f;
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueIndices = {
            _queueFamilies.graphicsFamily.value(),
            _queueFamilies.presentFamily.value()
        };

        for (uint32_t index : uniqueIndices) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = index;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;

            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE; 

        VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeature{};
        dynamicRenderingFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
        dynamicRenderingFeature.dynamicRendering = VK_TRUE;

        VkDeviceCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        info.pQueueCreateInfos = queueCreateInfos.data();              
        info.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        info.pEnabledFeatures = &deviceFeatures;
        info.pNext = &dynamicRenderingFeature;

        info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
        info.ppEnabledExtensionNames = device_extensions.data();

        if (vkCreateDevice(_physical_device, &info, nullptr, &_device) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create logical device");
        }

        vkGetDeviceQueue(_device, _queueFamilies.graphicsFamily.value(), 0, &_graphicsQueue);
        vkGetDeviceQueue(_device, _queueFamilies.presentFamily.value(), 0, &_presentQueue);

        volkLoadDevice(_device);
    }

    void vk_device::createAllocator(vk_context& context)
    {
        VmaVulkanFunctions vkFuncs{};
        vkFuncs.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
        vkFuncs.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

        VmaAllocatorCreateInfo info{};
        info.vulkanApiVersion = VK_API_VERSION_1_3;
        info.physicalDevice = _physical_device;
        info.device = _device;
        info.instance = context.vk_instance();
        info.pVulkanFunctions = &vkFuncs;

        if (vmaCreateAllocator(&info, &context.vk_allocator()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create VMA allocator");
        }
    }

    QueueFamilyIndices vk_device::findQueueFamilies(vk_context& context)
    {
        QueueFamilyIndices indices;

        uint32_t count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(_physical_device, &count, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(count);
        vkGetPhysicalDeviceQueueFamilyProperties(_physical_device, &count, queueFamilies.data());

        for (uint32_t i = 0; i < count; ++i)
        {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                indices.graphicsFamily = i;

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(_physical_device, i, context.vk_surface(), &presentSupport);
            if (presentSupport)
                indices.presentFamily = i;

            if (indices.isComplete())
                break;
        }

        return indices;
    }

    // validations
    bool checkDeviceExtensionSupport(VkPhysicalDevice& device, std::vector<const char*> extensions);

    bool vk_device::isDeviceSuitable(VkPhysicalDevice& device, vk_context& context) {
        return checkDeviceExtensionSupport(device, device_extensions);
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice& device, std::vector<const char*> extensions) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> available(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, available.data());

        for (const char* ext : extensions) {
            bool found = false;
            for (const auto& avail : available) {
                if (strcmp(ext, avail.extensionName) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found)
                return false;
        }

        return true;
    }

} // namespace vk
