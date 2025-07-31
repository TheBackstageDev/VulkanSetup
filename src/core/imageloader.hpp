#pragma once

#include <string>

#define NOMINMAX

#include "vk/vk_context.hpp"
#include "vk/vk_device.hpp"
#include "vk/vk_buffer.hpp"

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
        VmaAllocation allocation = VK_NULL_HANDLE;
    };
    
    class imageloader_t
    {
    public:
        static void loadImage(const std::string& path, image_t* pImage, std::unique_ptr<vk::vk_device>& device);
    private:
        static void createImage(image_t* image);
        static void createImageView(image_t* image);
        static VkSampler createSampler(VkFilter filter, VkSamplerAddressMode addressMode);
        static void createMipMaps(image_t* image);
        static void transitionImageLayout(image_t* image, VkImageLayout oldLayout, VkImageLayout newLayout);
    };
} // namespace core
