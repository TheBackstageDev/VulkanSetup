#pragma once

#include <volk/volk.h>
#include <vma/vk_mem_alloc.h>
#include <vector>

#include "vk_context.hpp"

namespace vk
{
    class vk_offscreen_renderer
    {
    public:
        vk_offscreen_renderer(size_t imageCount, VkExtent2D extent);
        ~vk_offscreen_renderer();

        vk_offscreen_renderer(const vk_offscreen_renderer&) = delete;
        vk_offscreen_renderer& operator=(const vk_offscreen_renderer&) = delete;

        VkCommandBuffer beginFrame();
        void endFrame(VkCommandBuffer cmd);

        float aspectRatio() const { return static_cast<float>(_extent.width / _extent.height); } 

        void recreate(VkExtent2D newExtent);

        VkImage getImage() { return _images[_imageIndex]; }
        VkImageView getImageView() { return _imageViews[_imageIndex]; }
    private:
        void beginRenderpass(VkCommandBuffer cmd);
        void endRenderpass(VkCommandBuffer cmd);

        VkCommandBuffer currentCommandBuffer() const { return _commandBuffers[_imageIndex]; }
        void createCommandBuffers();
        void freeCommandBuffers();

        void createImages();
        void recreateImages();

        void createDepthResources();
        void recreateDepthResources();

        std::vector<VkImage> _images;
        std::vector<VkImage> _depthImages;
        std::vector<VkImageView> _imageViews;
        std::vector<VkImageView> _depthImageViews;

        std::vector<VmaAllocation> _imageAllocations;
        std::vector<VmaAllocation> _depthImageAllocations;

        std::vector<VkCommandBuffer> _commandBuffers;
        uint32_t _imageIndex = 0;

        VkExtent2D _extent;
        size_t _imageCount;
    };
} // namespace vk
