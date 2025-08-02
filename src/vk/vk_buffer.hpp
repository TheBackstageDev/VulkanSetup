#pragma once

#include <volk/volk.h>
#include <vk_mem_alloc.h>

#include "vk_device.hpp"
#include "vk_context.hpp"
#include <memory>

namespace vk
{
    class vk_buffer
    {
    public:
        vk_buffer() = default;
        vk_buffer(const void* data, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
        ~vk_buffer();

        void* mapped() const { return _mapped; }
        VkBuffer buffer() { return _buffer; }

        void update(const void* newData)
        {
            memcpy(_mapped, newData, static_cast<size_t>(_size));
        }

        void bindUniform(VkCommandBuffer cmd, VkPipelineLayout layout, 
                        std::unique_ptr<vk_device>& _device, 
                        std::pair<uint32_t, uint32_t>& channelInfo)
        {
            vkCmdBindDescriptorSets(
                cmd,                            
                VK_PIPELINE_BIND_POINT_GRAPHICS,   
                layout,                     
                0,                               
                1,                          
                &_device->getDescriptorSet(channelInfo.first), 
                0,                                 
                &channelInfo.second         
            ); 
        }
    private:
        void* _mapped = nullptr;

        VkDeviceSize _size;    

        VkBuffer _buffer = VK_NULL_HANDLE;
        VmaAllocation _allocation = VK_NULL_HANDLE;
    };
     
} // namespace vk
