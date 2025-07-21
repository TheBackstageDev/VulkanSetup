#include "vk_swapchain.hpp"

#include <iostream>
#include <numeric>

namespace vk
{
    VkFormat vk_swapchain::_imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
    VkFormat vk_swapchain::_depthFormat = VK_FORMAT_D32_SFLOAT;

    vk_swapchain::vk_swapchain(std::unique_ptr<vk_device>& device, vk_context& context)
        : _device(device), _context(context)
    {
        init();
    }

    vk_swapchain::vk_swapchain(std::unique_ptr<vk_device>& device, vk_context& context, std::shared_ptr<vk_swapchain>& oldSwapchain)
        : _device(device), _context(context), _oldswapchain(oldSwapchain)
    {
        init();
        oldSwapchain = nullptr;
    }
    
    void vk_swapchain::init()
    {
        createSwapchain();
        createImageViews();
        createDepthImageViews();
        createSynchronizationObjects();
        createCommandPool();
        createRenderpass();
        createFramebuffers();
    }

    vk_swapchain::~vk_swapchain()
    {
        for (size_t i = 0; i < _images.size(); ++i)
        {
            vkDestroyImage(_device->device(), _images[i], nullptr);

            vkDestroyImage(_device->device(), _depthImages[i], nullptr);
            vmaDestroyImage(_context.vk_allocator(), _depthImages[i], _depthImagesAllocations[i]);

            vkDestroyImageView(_device->device(), _imageViews[i], nullptr);
            vkDestroyImageView(_device->device(), _depthImageViews[i], nullptr);
        }

        for (size_t i = 0; i < _frameSync.size(); ++i) 
        {
            FrameSync& contents = _frameSync[i];
            vkDestroySemaphore(_device->device(), contents.imageAvailable, nullptr);
            vkDestroySemaphore(_device->device(), contents.renderFinished, nullptr);
            vkDestroyFence(_device->device(), _imagesInFlight[i], nullptr);
            vkDestroyFence(_device->device(), contents.inFlightFence, nullptr);
        }

        vkDestroyCommandPool(_device->device(), _commandPool, nullptr);
        vkDestroySwapchainKHR(_device->device(), _swapchain, nullptr);
    }

    VkResult vk_swapchain::acquireNextImage(uint32_t* imageIndex)
    {
        vkWaitForFences(_device->device(), 1, &_frameSync[currentFrame].inFlightFence, VK_TRUE, UINT64_MAX);

        return vkAcquireNextImageKHR(
            _device->device(),
            _swapchain,
            UINT64_MAX,
            _frameSync[currentFrame].imageAvailable, 
            VK_NULL_HANDLE,
            imageIndex);
    }

    VkResult vk_swapchain::submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex)
    {
        uint32_t index = *imageIndex;

        if (_imagesInFlight[index] != VK_NULL_HANDLE) {
            vkWaitForFences(_device->device(), 1, &_imagesInFlight[index], VK_TRUE, UINT64_MAX);
        }

        _imagesInFlight[index] = _frameSync[currentFrame].inFlightFence;

        VkSemaphore waitSemaphores[]    = { _frameSync[currentFrame].imageAvailable };
        VkSemaphore signalSemaphores[]  = { _frameSync[index].renderFinished };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSubmitInfo submitInfo{};
        submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount   = 1;
        submitInfo.pWaitSemaphores      = waitSemaphores;
        submitInfo.pWaitDstStageMask    = waitStages;
        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = buffers;
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = signalSemaphores;

        vkResetFences(_device->device(), 1, &_frameSync[currentFrame].inFlightFence);
        if (vkQueueSubmit(_device->graphicsQueue(), 1, &submitInfo, _frameSync[currentFrame].inFlightFence) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = signalSemaphores;
        presentInfo.swapchainCount     = 1;
        presentInfo.pSwapchains        = &_swapchain;
        presentInfo.pImageIndices      = imageIndex;

        VkResult result = vkQueuePresentKHR(_device->presentQueue(), &presentInfo);

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        return result;
    }

    void vk_swapchain::createSwapchain()
    {
        VkSurfaceCapabilitiesKHR capabilities;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device->phydevice(), _context.vk_surface(), &capabilities);

        VkSurfaceFormatKHR surfaceFormat = { VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
        VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
        _extent = capabilities.currentExtent;

        uint32_t imageCount = capabilities.minImageCount + 1;
        if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
            imageCount = capabilities.maxImageCount;

        VkSwapchainCreateInfoKHR info{};
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info.surface = _context.vk_surface();
        info.minImageCount = imageCount;
        info.imageFormat = surfaceFormat.format;
        info.imageColorSpace = surfaceFormat.colorSpace;
        info.imageExtent = _extent;
        info.imageArrayLayers = 1;
        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        auto graphicsFamily = _device->graphicsFamily();
        auto presentFamily = _device->presentFamily();

        uint32_t queueFamilyIndices[] = {graphicsFamily, presentFamily};

        if (graphicsFamily != presentFamily)
        {
            info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            info.queueFamilyIndexCount = 2;
            info.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            info.queueFamilyIndexCount = 0;     // Optional
            info.pQueueFamilyIndices = nullptr; // Optional
        }

        info.preTransform = capabilities.currentTransform;
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info.presentMode = presentMode;
        info.clipped = VK_TRUE;
        info.oldSwapchain = _oldswapchain ? _oldswapchain->_swapchain : VK_NULL_HANDLE;

        _imageFormat = surfaceFormat.format;
        _depthFormat = VK_FORMAT_D32_SFLOAT;

        if (vkCreateSwapchainKHR(_device->device(), &info, nullptr, &_swapchain) != VK_SUCCESS)
            throw std::runtime_error("Failed to create swapchain");
        
        vkGetSwapchainImagesKHR(_device->device(), _swapchain, &imageCount, nullptr);
        _images.resize(imageCount);
        vkGetSwapchainImagesKHR(_device->device(), _swapchain, &imageCount, _images.data());
    }

    void vk_swapchain::createCommandPool()
    {
        VkCommandPoolCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        info.queueFamilyIndex = _device->graphicsFamily();

        if (vkCreateCommandPool(_device->device(), &info, nullptr, &_commandPool) != VK_SUCCESS)
            throw std::runtime_error("Failed to create command pool!");
    }

    void vk_swapchain::createRenderpass()
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = _imageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = _depthFormat;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        VkAttachmentDescription attachments[] = {
            colorAttachment, depthAttachment
        };

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 2;
        renderPassInfo.pAttachments = attachments;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        vkCreateRenderPass(_device->device(), &renderPassInfo, nullptr, &_renderpass);
    }

    void vk_swapchain::createFramebuffers()
    {
        _framebuffers.resize(_imageViews.size());

        for (size_t i = 0; i < _imageViews.size(); ++i) {
            VkImageView attachments[] = {
                _imageViews[i],
                _depthImageViews[i]
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = _renderpass; 
            framebufferInfo.attachmentCount = 2;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = _extent.width;
            framebufferInfo.height = _extent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(_device->device(), &framebufferInfo, nullptr, &_framebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create framebuffer!");
            }
        }
    }

    void vk_swapchain::createImageViews()
    {
        _imageViews.resize(_images.size());

        for (size_t i = 0; i < _images.size(); ++i)
        {
            VkImageViewCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            info.image = _images[i];
            info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            info.format = _imageFormat;

            info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            info.subresourceRange.baseMipLevel = 0;
            info.subresourceRange.levelCount = 1;
            info.subresourceRange.baseArrayLayer = 0;
            info.subresourceRange.layerCount = 1;

            if (vkCreateImageView(_device->device(), &info, nullptr, &_imageViews[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create image view for swapchain image!");
            }
        }
    }

    void vk_swapchain::createDepthImageViews()
    {
        _depthImages.resize(_images.size());
        _depthImageViews.resize(_images.size());
        _depthImagesAllocations.resize(_images.size());

        for (size_t i = 0; i < _images.size(); ++i)
        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = _extent.width;
            imageInfo.extent.height = _extent.height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = _depthFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo allocInfo{};
            allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
            allocInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

            if (vmaCreateImage(_context.vk_allocator(), &imageInfo, &allocInfo, &_depthImages[i], &_depthImagesAllocations[i], nullptr) != VK_SUCCESS)
                throw std::runtime_error("Failed to create depth image");

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = _depthImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = _depthFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(_device->device(), &viewInfo, nullptr, &_depthImageViews[i]) != VK_SUCCESS)
                throw std::runtime_error("Failed to create depth image view");
        }
    }

    void vk_swapchain::createSynchronizationObjects()
    {
        _frameSync.resize(imageAmmount());
        _imagesInFlight.resize(imageAmmount(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; 

        for (size_t i = 0; i < imageAmmount(); ++i)
        {
            if (vkCreateSemaphore(_device->device(), &semaphoreInfo, nullptr, &_frameSync[i].imageAvailable) != VK_SUCCESS ||
                vkCreateSemaphore(_device->device(), &semaphoreInfo, nullptr, &_frameSync[i].renderFinished) != VK_SUCCESS ||
                vkCreateFence(_device->device(), &fenceInfo, nullptr, &_frameSync[i].inFlightFence) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create sync objects for swapchain image");
            }
        }
    }
} // namespace vk
