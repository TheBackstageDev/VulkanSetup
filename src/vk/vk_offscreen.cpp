#include "vk_offscreen.hpp"

namespace vk
{
    vk_offscreen_renderer::vk_offscreen_renderer(size_t imageCount, VkExtent2D extent)
        : _imageCount(imageCount), _extent(extent)
    {
        createCommandBuffers();
        createImages();
        createDepthResources();
    }

    vk_offscreen_renderer::~vk_offscreen_renderer()
    {
        vkDeviceWaitIdle(vk_context::device);
        freeCommandBuffers();

        for (size_t i = 0; i < _imageCount; ++i)
        {
            vmaDestroyImage(vk_context::allocator, _images[i], _imageAllocations[i]);
            vmaDestroyImage(vk_context::allocator, _depthImages[i], _depthImageAllocations[i]);
            vkDestroyImageView(vk_context::device, _imageViews[i], nullptr);
            vkDestroyImageView(vk_context::device, _depthImageViews[i], nullptr);
        }
    }

    VkCommandBuffer vk_offscreen_renderer::beginFrame()
    {
        _imageIndex = (_imageIndex % _imageCount);
        VkCommandBuffer cmd = currentCommandBuffer();

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to begin command buffer for offscreen rendering!");
        }

        beginRenderpass(cmd);
        return cmd;
    }

    void vk_offscreen_renderer::endFrame(VkCommandBuffer cmd)
    {
        endRenderpass(cmd);

        if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to end command buffer for offscreen rendering!");
        }
    }

    void vk_offscreen_renderer::beginRenderpass(VkCommandBuffer cmd)
    {
        VkClearValue clearValues[2];
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };      // Clear color
        clearValues[1].depthStencil = { 1.0f, 0 };                // Clear depth

        VkRenderingAttachmentInfo colorAttachment{};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageView = _imageViews[_imageIndex]; 
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.clearValue = clearValues[0];

        VkRenderingAttachmentInfo depthAttachment{};
        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachment.imageView = _depthImageViews[_imageIndex];
        depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.clearValue = clearValues[1];

        VkRenderingInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea.offset = { 0, 0 };
        renderingInfo.renderArea.extent = _extent; // Example extent
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;
        renderingInfo.pDepthAttachment = &depthAttachment;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(_extent.width);
        viewport.height = static_cast<float>(_extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{ {0, 0}, _extent };
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = _images[_imageIndex];
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,               // srcStageMask
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,   // dstStageMask
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        vkCmdBeginRenderingKHR(cmd, &renderingInfo);
    }

    void vk_offscreen_renderer::endRenderpass(VkCommandBuffer cmd)
    {
        vkCmdEndRenderingKHR(cmd);

        VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = _images[_imageIndex];
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; 
            barrier.dstAccessMask = 0;

        vkCmdPipelineBarrier(
            cmd,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,          
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);
    }

    void vk_offscreen_renderer::freeCommandBuffers()
    {
        vkFreeCommandBuffers(
            vk_context::device, 
            vk_context::commandPool, 
            static_cast<uint32_t>(_commandBuffers.size()), 
            _commandBuffers.data()
        );

        _commandBuffers.clear();
    }

    void vk_offscreen_renderer::createCommandBuffers()
    {
        _commandBuffers.resize(_imageCount);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = vk_context::commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(_imageCount);

        if (vkAllocateCommandBuffers(vk_context::device, &allocInfo, _commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate command buffers for offscreen rendering!");
        }
    }

    void vk_offscreen_renderer::recreate(VkExtent2D newExtent)
    {
        _extent = newExtent;

        vkDeviceWaitIdle(vk_context::device);

        freeCommandBuffers();
        recreateImages();
        recreateDepthResources();
        createCommandBuffers();
    }

    void vk_offscreen_renderer::recreateImages()
    {
        for (size_t i = 0; i < _imageCount; ++i)
        {
            vmaDestroyImage(vk_context::allocator, _images[i], _imageAllocations[i]);
            vkDestroyImageView(vk_context::device, _imageViews[i], nullptr);
        }

        createImages();
    }

    void vk_offscreen_renderer::recreateDepthResources()
    {
        for (size_t i = 0; i < _imageCount; ++i)
        {
            vmaDestroyImage(vk_context::allocator, _depthImages[i], _depthImageAllocations[i]);
            vkDestroyImageView(vk_context::device, _depthImageViews[i], nullptr);
        }

        createDepthResources();
    }

    void vk_offscreen_renderer::createImages()
    {
        _images.resize(_imageCount);
        _imageAllocations.resize(_imageCount);
        _imageViews.resize(_imageCount);

        for (size_t i = 0; i < _imageCount; ++i)
        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.format = vk_context::imageFormat;
            imageInfo.extent.width = 800; // Example width
            imageInfo.extent.height = 600; // Example height
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

            VmaAllocationCreateInfo allocCreateInfo{};
            allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            if (vmaCreateImage(vk_context::allocator, &imageInfo, &allocCreateInfo, &_images[i], &_imageAllocations[i], nullptr) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create offscreen images!");
            }

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = _images[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = vk_context::imageFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(vk_context::device, &viewInfo, nullptr, &_imageViews[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create offscreen image views!");
            }
        }
    }

    void vk_offscreen_renderer::createDepthResources()
    {
        _depthImages.resize(_imageCount);
        _depthImageAllocations.resize(_imageCount);
        _depthImageViews.resize(_imageCount);

        for (size_t i = 0; i < _imageCount; ++i)
        {
            VkImageCreateInfo depthImageInfo{};
            depthImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            depthImageInfo.imageType = VK_IMAGE_TYPE_2D;
            depthImageInfo.format = vk_context::depthFormat;
            depthImageInfo.extent.width = 800; // Example width
            depthImageInfo.extent.height = 600; // Example height
            depthImageInfo.extent.depth = 1;
            depthImageInfo.mipLevels = 1;
            depthImageInfo.arrayLayers = 1;
            depthImageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            depthImageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            depthImageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

            VmaAllocationCreateInfo allocCreateInfo{};
            allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            if (vmaCreateImage(vk_context::allocator, &depthImageInfo, &allocCreateInfo, &_depthImages[i], &_depthImageAllocations[i], nullptr) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create offscreen depth images!");
            }

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = _depthImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = vk_context::depthFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(vk_context::device, &viewInfo, nullptr, &_depthImageViews[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create offscreen depth image views!");
            }
        }
    }


} // namespace vk
