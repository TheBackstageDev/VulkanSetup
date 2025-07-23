#pragma once

#include "vk_context.hpp"
#include "vk_device.hpp"

#include <memory>
#include <vector>
#include <vma/vk_mem_alloc.h>

namespace vk
{
    struct FrameSync
    {
        VkSemaphore imageAvailable = VK_NULL_HANDLE;
        VkSemaphore renderFinished = VK_NULL_HANDLE;
        VkFence     inFlightFence  = VK_NULL_HANDLE;
    };

    class vk_swapchain
    {
    public:
        vk_swapchain(std::unique_ptr<vk_device>& device, vk_context& context);
        vk_swapchain(std::unique_ptr<vk_device>& device, vk_context& context, std::shared_ptr<vk_swapchain>& oldSwapchain);

        ~vk_swapchain();

        VkResult acquireNextImage(uint32_t *imageIndex);

        VkSwapchainKHR swapchain() { return _swapchain; }
        const std::vector<VkImageView>& imageViews() const { return _imageViews; }

        static constexpr size_t getMaxFramesInFlights() { return MAX_FRAMES_IN_FLIGHT; }

        VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);
        
        void beginCommandBuffers();
        void endCommandBuffers();

        static VkFormat imageFormat() { return _imageFormat; }
        static VkFormat depthFormat() { return _depthFormat; }
        VkExtent2D extent() const { return _extent; }

        VkImageView imageView(uint32_t imageIndex) { return _imageViews[imageIndex]; }
        VkImage image(uint32_t imageIndex) { return _images[imageIndex]; }

        VkImageView depthImageView(uint32_t imageIndex) { return _depthImageViews[imageIndex]; }
        VkImage depthImage(uint32_t imageIndex) { return _depthImages[imageIndex]; }
        VkCommandPool commandPool() { return _commandPool; }

        uint32_t imageAmmount() { return _images.size(); }
        size_t getCurrentFrame() { return _currentFrame; }

    private:
        void createSwapchain();
        void createImageViews();
        void createDepthImageViews();
        void createCommandPool();
        void createSynchronizationObjects();
        void createRenderpass();
        void createFramebuffers();

        void init();

        // validations

        bool formatAvailable(VkFormat format);

        uint32_t _currentFrame = 0;
        static constexpr size_t MAX_FRAMES_IN_FLIGHT = 2;

        VkSwapchainKHR _swapchain = VK_NULL_HANDLE;
        VkCommandPool _commandPool = VK_NULL_HANDLE;

        std::vector<VkImage> _images;
        std::vector<VkImage> _depthImages;
        std::vector<VkImageView> _imageViews;
        std::vector<VkImageView> _depthImageViews;
        
        std::vector<VmaAllocation> _depthImagesAllocations;

        std::vector<FrameSync> _frameSync;
        std::vector<VkFence> _imagesInFlight;

        static VkFormat _imageFormat;
        static VkFormat _depthFormat;
        VkExtent2D _extent;

        vk_context& _context;
        std::unique_ptr<vk_device>& _device;
        std::shared_ptr<vk_swapchain> _oldswapchain;

        size_t currentFrame = 0;
    };
} // namespace vk
