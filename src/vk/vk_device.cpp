#include "vk_device.hpp"
#include <iostream>
#include <set>
#include <string>

namespace vk
{
    VkPhysicalDeviceProperties vk_device::_properties;
    
    vk_device::vk_device(vk_context& context)
        : context(context)
    {
        pickPhydevice(context);
        createDevice(context);
        createAllocator(context);

        createDescriptorPools(context);
        allocateSets();
        createResourceChannels(context);
    }

    vk_device::~vk_device()
    {
        vkDestroyDescriptorPool(_device, _uniformDescriptorPool, nullptr);
        vkDestroyDescriptorPool(_device, _SSBOdescriptorPool, nullptr);
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

        VkPhysicalDeviceBufferDeviceAddressFeatures bdaFeatures{};
        bdaFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
        bdaFeatures.bufferDeviceAddress = VK_TRUE;
        bdaFeatures.bufferDeviceAddressCaptureReplay = VK_FALSE; // optional
        bdaFeatures.bufferDeviceAddressMultiDevice = VK_FALSE;   // optional

        VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
        indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
        indexingFeatures.runtimeDescriptorArray = VK_TRUE;
        indexingFeatures.descriptorBindingPartiallyBound = VK_TRUE;
        indexingFeatures.descriptorBindingVariableDescriptorCount = VK_TRUE;
        indexingFeatures.shaderStorageBufferArrayNonUniformIndexing = VK_TRUE;
        indexingFeatures.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        indexingFeatures.shaderStorageImageArrayNonUniformIndexing = VK_TRUE;
        indexingFeatures.pNext = &bdaFeatures;
        
        VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeature{};
        dynamicRenderingFeature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES;
        dynamicRenderingFeature.dynamicRendering = VK_TRUE;
        dynamicRenderingFeature.pNext = &indexingFeatures;

        vkGetPhysicalDeviceProperties(_physical_device, &_properties);

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

        vk::vk_context::graphicsQueue = _graphicsQueue;
        vk::vk_context::presentQueue = _presentQueue;

        volkLoadDevice(_device);
        context.device = _device;
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

        if (vmaCreateAllocator(&info, &context.allocator) != VK_SUCCESS) {
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

    void vk_device::createDescriptorPools(vk_context& context)
    {
        VkDescriptorPoolSize uniformSize{};
        uniformSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uniformSize.descriptorCount = _properties.limits.maxDescriptorSetUniformBuffers;

        VkDescriptorPoolCreateInfo uniformPoolInfo{};
        uniformPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        uniformPoolInfo.maxSets = numUniform;
        uniformPoolInfo.poolSizeCount = 1;
        uniformPoolInfo.pPoolSizes = &uniformSize;
        uniformPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

        VkDescriptorPoolSize SSBOSizes[] = {
            {
                .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = _properties.limits.maxDescriptorSetSampledImages,
            },
            {
                .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = _properties.limits.maxDescriptorSetStorageBuffers,
            }
        };

        VkDescriptorPoolCreateInfo ssboPoolInfo{};
        ssboPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        ssboPoolInfo.maxSets = numSSBO + numCombinedImageSampler;
        ssboPoolInfo.poolSizeCount = 2;
        ssboPoolInfo.pPoolSizes = SSBOSizes;
        ssboPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;

        if (vkCreateDescriptorPool(_device, &uniformPoolInfo, nullptr, &_uniformDescriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create uniform descriptor pool");
        }

        if (vkCreateDescriptorPool(_device, &ssboPoolInfo, nullptr, &_SSBOdescriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create SSBO descriptor pool");
        }
    }

    void vk_device::allocateSets()
    {
        _sets.resize(numChannels, VK_NULL_HANDLE);
        _setLayouts.resize(numChannels, VK_NULL_HANDLE);

        for (int32_t i = 0; i < numChannels; ++i)
        {
            VkDescriptorSetLayoutBinding binding{};
            binding.binding = 0;
            binding.descriptorCount = 1;

            if (i < numUniform) {
                binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            } else if (i < numUniform + numSSBO) {
                binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            } else {
                binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            }
            binding.stageFlags = i < numChannels - numCombinedImageSampler 
                ? VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT : VK_SHADER_STAGE_FRAGMENT_BIT;
            binding.pImmutableSamplers = nullptr;

            VkDescriptorSetLayoutCreateInfo layoutInfo{};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = 1;
            layoutInfo.pBindings = &binding;

            if (vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr, &_setLayouts[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create descriptor set layout at index " + std::to_string(i));
            }
        }

        for (int32_t i = 0; i < numChannels; ++i)
        {
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = i < numUniform ? _uniformDescriptorPool : _SSBOdescriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &_setLayouts[i];

            if (vkAllocateDescriptorSets(_device, &allocInfo, &_sets[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to allocate descriptor set " + std::to_string(i));
            }
        }
    }

    void vk_device::createResourceChannels(vk_context& context)
    {
        _channels.reserve(numChannels);

        for (int32_t i = 0; i < numChannels ; ++i)
        {
            VkDescriptorType type = (i < numUniform) ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER : 
                                    (i < numUniform + numSSBO) ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER : 
                                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

            vk_resourcechannel channel{
                *this,
                static_cast<uint32_t>(i),
                (i < numUniform)
                    ? _properties.limits.maxDescriptorSetUniformBuffers
                    : _properties.limits.maxPerStageDescriptorStorageBuffers,
                type
            };

            _channels.push_back(std::move(channel));
        }
    }

    // Returns the channel and index of the data which was set;
    std::pair<uint32_t, uint32_t> vk_device::setDescriptorData(vk_descriptordata& data, uint32_t channel, uint32_t index)
    {
        if (channel != UINT32_MAX)
        {
            auto& _channel = _channels[channel];
            _channel.gotoIndex(index);
            _channel.bind(data);

            return std::pair<uint32_t, uint32_t>{channel, index};
        }

        uint32_t channelId = 0;
        uint32_t newIndex = 0;

        int32_t start, end;

        if (data.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
        {
            start = 0;
            end = numUniform;
        }
        else if (data.type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
        {
            start = numUniform;
            end = numUniform + numSSBO;
        }
        else // VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
        {
            start = numUniform + numSSBO;
            end = numChannels;
        }

        for (int32_t i = start; i < end; ++i)
        {
            auto& channel = _channels[i];
            channel.gotoNext();

            newIndex = channel.getIndex();
            if (newIndex != channel.getSize())
            {
                channelId = i;
                _channels[channelId].bind(data);

                break;
            }
        }

        return std::pair<uint32_t, uint32_t>{channelId, newIndex};
    }

    void vk_device::freeDescriptorData(uint32_t index, uint32_t channelId)
    {
        _channels[channelId].gotoIndex(index);
        _channels[channelId].free();
    }

    // resourceChannel

    vk_resourcechannel::vk_resourcechannel(vk_device& device, uint32_t channelId, size_t size, VkDescriptorType type)
        : _device(device), _channelId(channelId), _maxIndices(size), _type(type)
    {
        freeIndices.reserve(_maxIndices);
    }

    void vk_resourcechannel::gotoNext()
    {
        if (!freeIndices.empty()) {
            _index = freeIndices.back();
            freeIndices.pop_back();
        } else if (_countIndices < _maxIndices) {
            _index = _countIndices++;
        } else {
            _index = _maxIndices;
        }
    }

    void vk_resourcechannel::gotoCurrent()
    {
        if (_countIndices < _maxIndices) {
            _index = _countIndices;
        } else {
            _index = _maxIndices; 
        }
    }

    void vk_resourcechannel::bind(const vk_descriptordata& data)
    {
        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = _device._sets[_channelId];
        write.dstBinding = 0;
        write.dstArrayElement = _index;        
        write.descriptorCount = 1;
        write.descriptorType = _type;

        if (_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || 
            _type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
            write.pBufferInfo = data.pBufferInfo;
        } else /* VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER */ {
            write.pImageInfo = data.pImageInfo;
        }

        vkUpdateDescriptorSets(_device.device(), 1, &write, 0, nullptr);
        
        --_countIndices;
    }

    void vk_resourcechannel::free()
    {
        freeIndices.push_back(_index);
        _index = -1;
    }

} // namespace vk
