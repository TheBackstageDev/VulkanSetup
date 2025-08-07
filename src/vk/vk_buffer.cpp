#include "vk_buffer.hpp"

#include <iostream>

namespace vk
{
    vk_buffer::vk_buffer(const void* data, 
    VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
        : _size(size)
    {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = memoryUsage;

        if (vmaCreateBuffer(vk_context::allocator, &bufferInfo, &allocInfo, &_buffer, &_allocation, nullptr) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create Vulkan buffer");
        }

        if (data && (memoryUsage == VMA_MEMORY_USAGE_CPU_ONLY || memoryUsage == VMA_MEMORY_USAGE_CPU_TO_GPU))
        {
            if (vmaMapMemory(vk_context::allocator, _allocation, &_mapped) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to map Vulkan buffer memory");
            }

            memcpy(_mapped, data, static_cast<size_t>(size));
            vmaUnmapMemory(vk_context::allocator, _allocation);
        }
    }

    vk_buffer::~vk_buffer()
    {
        vmaDestroyBuffer(vk_context::allocator, _buffer, _allocation);
    }
} // namespace vk
