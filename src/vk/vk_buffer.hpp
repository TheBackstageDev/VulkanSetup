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

        void* mapped() { return _mapped; }
        VkBuffer buffer() { return _buffer; }
    private:
        void* _mapped = nullptr;

        VkBuffer _buffer = VK_NULL_HANDLE;
        VmaAllocation _allocation = VK_NULL_HANDLE;
    };
     
} // namespace vk
