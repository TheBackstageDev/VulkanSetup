#pragma once

#include <volk/volk.h>
#include <vk_mem_alloc.h>

#include "vk_device.hpp"
#include <memory>

namespace vk
{
    class vk_buffer
    {
    public:
        vk_buffer() = default;
        vk_buffer(std::unique_ptr<vk_device>& device, const void* data, VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
        ~vk_buffer();

        void* mapped() const { return _mapped; }
        VkBuffer buffer() { return _buffer; }

        void update(const void* newData)
        {
            memcpy(_mapped, newData, static_cast<size_t>(_size));
        }
    private:
        void* _mapped = nullptr;

        VkDeviceSize _size;    

        VkBuffer _buffer = VK_NULL_HANDLE;
        VmaAllocation _allocation = VK_NULL_HANDLE;
    };
     
} // namespace vk
