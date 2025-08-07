#pragma once

#include <volk/volk.h>
#include <vma/vk_mem_alloc.h>
#include <vector>

#include "vk_context.hpp"
#include "vK_device.hpp"

namespace vk
{
    class vk_offscreen_renderer
    {
    public:
        vk_offscreen_renderer(size_t imageCount, VkExtent2D extent);
        ~vk_offscreen_renderer();

        vk_offscreen_renderer(const vk_offscreen_renderer&) = delete;
        vk_offscreen_renderer& operator=(const vk_offscreen_renderer&) = delete;

        void createNextImage();
        void beginRenderpass(VkCommandBuffer cmd);
        void endRenderpass(VkCommandBuffer cmd);
        
        void recreate(VkExtent2D newExtent);
        
        float aspectRatio() const { return static_cast<float>(_extent.width / _extent.height); } 
        VkImage getImage() { return _images[_imageIndex]; }
        VkImageView getImageView() { return _imageViews[_imageIndex]; }
        VkSampler getSampler() { return _sampler; }
    private:
        void createImages();
        void recreateImages();

        void createDepthResources();
        void recreateDepthResources();

        void createSampler();

        std::vector<VkImage> _images;
        std::vector<VkImage> _depthImages;
        std::vector<VkImageView> _imageViews;
        std::vector<VkImageView> _depthImageViews;

        std::vector<VmaAllocation> _imageAllocations;
        std::vector<VmaAllocation> _depthImageAllocations;

        VkSampler _sampler = VK_NULL_HANDLE;

        uint32_t _imageIndex = 0;

        VkExtent2D _extent;
        size_t _imageCount;
    };
} // namespace vk
