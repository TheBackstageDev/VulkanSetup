#pragma once

#include <volk/volk.h>
#include "vk_context.hpp"

#include <optional>
#include <array>
#include <vector>

namespace vk
{
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct vk_descriptordata
    {
        VkDescriptorType type;
        
        union {
            VkDescriptorBufferInfo* pBufferInfo;
            VkDescriptorImageInfo* pImageInfo;
        }; 
    };

    struct vk_channelindices
    {
        uint32_t channelIndex = UINT32_MAX;
        uint32_t index = UINT32_MAX;
    };

    struct vk_channelinfo
    {
        std::vector<uint32_t> uniformIndices;
        std::vector<uint32_t> SSBOIndices;
        std::vector<uint32_t> combinedImageSamplerIndices;
    };

    class vk_resourcechannel;

    class vk_device
    {
    public:
        vk_device(vk_context& context);
        ~vk_device();

        const VkDevice& device() { return _device; }
        const VkPhysicalDevice phydevice() { return _physical_device; }
        
        uint32_t graphicsFamily() const { return _queueFamilies.graphicsFamily.value(); }
        uint32_t presentFamily() const { return _queueFamilies.presentFamily.value(); }

        // Returns the channel and index of the data which was set;
        vk_channelindices setDescriptorData(vk_descriptordata& data, uint32_t channel = -1 /* channel if you alreadly have one */, uint32_t index = -1);
        void freeDescriptorData(uint32_t index, uint32_t channelId);

        VkDescriptorSet& getDescriptorSet(uint32_t channelId) { return _sets[channelId]; }
        std::vector<VkDescriptorSetLayout> getSetLayouts() const { return _setLayouts; }
        
        static VkPhysicalDeviceLimits limits() { return _properties.limits; }

        constexpr vk_channelinfo getChannelInfo() {
            vk_channelinfo channelInfo;
            channelInfo.uniformIndices.reserve(numUniform);
            channelInfo.SSBOIndices.reserve(numSSBO);
            channelInfo.combinedImageSamplerIndices.reserve(numCombinedImageSampler);

            for (uint32_t i = 0; i < numChannels; ++i)
            {
                if (i < numUniform)
                {
                    channelInfo.uniformIndices.push_back(i);
                }
                else if (i < numUniform + numSSBO)
                {
                    channelInfo.SSBOIndices.push_back(i);
                }
                else
                {
                    channelInfo.combinedImageSamplerIndices.push_back(i);
                }
            }

            return channelInfo;
        }
    private:
        friend vk_resourcechannel;

        void pickPhydevice(vk_context& context);
        void createDevice(vk_context& context);
        void createAllocator(vk_context& context);
        void createDescriptorPools(vk_context& context); 
        void createResourceChannels(vk_context& context);
        void allocateSets();

        bool isDeviceSuitable(VkPhysicalDevice& device, vk_context& context);
        QueueFamilyIndices findQueueFamilies(vk_context& context);

        VkDevice _device = VK_NULL_HANDLE;
        VkPhysicalDevice _physical_device = VK_NULL_HANDLE;
        
        vk_context& context;

        QueueFamilyIndices _queueFamilies;

        const uint32_t numUniform = 1;
        const uint32_t numSSBO = 0;
        const uint32_t numCombinedImageSampler = 1;
        const uint32_t numChannels = numUniform + numSSBO + numCombinedImageSampler;

        VkDescriptorPool _uniformDescriptorPool;
        VkDescriptorPool _SSBOdescriptorPool;

        std::vector<VkDescriptorSet> _sets;
        std::vector<VkDescriptorSetLayout> _setLayouts;

        std::vector<vk_resourcechannel> _channels;

        static VkPhysicalDeviceProperties _properties; 

        std::vector<const char*> device_extensions = 
            {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME,
             VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME};
    };

    class vk_resourcechannel
    {
    public:
        vk_resourcechannel(vk_device& device, uint32_t channelId, size_t size, VkDescriptorType type);
        
        void bind(const vk_descriptordata& data);
        void free();

        void gotoCurrent();
        void gotoNext();
        void gotoIndex(uint32_t index) { _index = index; }

        size_t getSize() { return _maxIndices; }
        uint32_t getIndex() { return _index; }

        const VkDescriptorType _type;
    private:
        uint32_t _channelId = UINT32_MAX;
        uint32_t _index = UINT32_MAX;

        const size_t _maxIndices = 0;
        size_t _countIndices = 0;

        std::vector<uint32_t> freeIndices;

        vk_device& _device;
    };

} // namespace vk
