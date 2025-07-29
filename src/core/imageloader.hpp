#pragma once

#include <string>

#define NOMINMAX
#include <volk/volk.h>
#include <vk_mem_alloc.h>

namespace core
{
    struct image_t
    {
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t mipLevels = 1;

        VkImage image = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        VkSampler sampler = VK_NULL_HANDLE;
        VkFormat format = VK_FORMAT_UNDEFINED;
        VmaAllocation allocation;
    };
    
    class imageloader_t
    {
    public:
        static void loadImage(const std::string& path, image_t* pImage);
    private:
        static void createImage(image_t* image, VmaAllocator allocator);
        static void createImageView(image_t* image, VkDevice device);
        static VkSampler createSampler(VkDevice device, VkFilter filter, VkSamplerAddressMode addressMode);
        static void createMipMaps(image_t* image);
    };
} // namespace core
