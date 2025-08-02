#include "vk_buffer.hpp"

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

        vmaCreateBuffer(vk_context::allocator, &bufferInfo, &allocInfo, &_buffer, &_allocation, nullptr);

        if (memoryUsage == VMA_MEMORY_USAGE_CPU_ONLY || memoryUsage == VMA_MEMORY_USAGE_CPU_TO_GPU)
        {
            vmaMapMemory(vk_context::allocator, _allocation, &_mapped);
            memcpy(_mapped, data, static_cast<size_t>(size));
            vmaUnmapMemory(vk_context::allocator, _allocation);
        }
    }

    vk_buffer::~vk_buffer()
    {
        vmaDestroyBuffer(vk_context::allocator, _buffer, _allocation);
    }
} // namespace vk
