#include "vk_renderer.hpp"
#include <iostream>
#include <cassert>

namespace vk
{
    frameinfo_t vk_renderer::_info;

    vk_renderer::vk_renderer(
        std::unique_ptr<vk_pipeline>& pipeline, 
        std::shared_ptr<vk_swapchain>& swapchain, 
        std::unique_ptr<vk_device>& device, 
        vk_context& context, std::unique_ptr<vk_window>& window
    )    
        : pipeline(pipeline), swapchain(swapchain), device(device), context(context), window(window)
    {
        createCommandBuffers();
    }

    vk_renderer::~vk_renderer()
    {
        vkDeviceWaitIdle(device->device());
        freeCommandBuffers();
    }

    void vk_renderer::recreateSwapchain()
    {
        VkExtent2D extent = window->extent();

        while(extent.height == 0 || extent.width == 0)
        {
            extent = window->extent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device->device());

        if (nullptr == swapchain)
        {
            swapchain = std::make_shared<vk_swapchain>(device, context);
        }
        else
        {
            std::shared_ptr<vk_swapchain> oldswapchain = std::move(swapchain);
            swapchain = std::make_shared<vk_swapchain>(device, context, oldswapchain);

            if(swapchain->imageAmmount() != commandBuffers.size())
            {
                freeCommandBuffers();
                createCommandBuffers();
            }
        }

        window->resetResizedFlag();
        
        vkDeviceWaitIdle(device->device());
    }

    void vk_renderer::freeCommandBuffers()
    {
        vkFreeCommandBuffers(device->device(), swapchain->commandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        commandBuffers.clear();
    }

    void vk_renderer::createCommandBuffers()
    {
        commandBuffers.resize(swapchain->imageAmmount());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = swapchain->commandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(device->device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate command buffers!");
    }

    void vk_renderer::beginRenderpass(VkCommandBuffer cmd)
    {
        VkClearValue clearValues[2];
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };      // Clear color
        clearValues[1].depthStencil = { 1.0f, 0 };                // Clear depth

        VkRenderingAttachmentInfo colorAttachment{};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachment.imageView = swapchain->imageView(imageIndex); 
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.clearValue = clearValues[0];

        VkRenderingAttachmentInfo depthAttachment{};
        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachment.imageView = swapchain->depthImageView(imageIndex);
        depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.clearValue = clearValues[1];

        VkRenderingInfo renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        renderingInfo.renderArea.offset = { 0, 0 };
        renderingInfo.renderArea.extent = swapchain->extent();
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;
        renderingInfo.pDepthAttachment = &depthAttachment;
        renderingInfo.pStencilAttachment = nullptr;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(swapchain->extent().width);
        viewport.height = static_cast<float>(swapchain->extent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{ {0, 0}, swapchain->extent() };
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = swapchain->image(imageIndex);
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

    void vk_renderer::endRenderpass(VkCommandBuffer cmd)
    {
        vkCmdEndRenderingKHR(cmd);

        VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = swapchain->image(imageIndex);
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

    VkCommandBuffer vk_renderer::startFrame()
    {
        assert(!isFrameRunning && "Cannot start new frame while another is running!");
        VkResult result = swapchain->acquireNextImage(&imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || window->resized())
        {
            recreateSwapchain();
            return VK_NULL_HANDLE;
        }
            
        VkCommandBuffer cmd = currentCommandBuffer();
        vkResetCommandBuffer(cmd, 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS)
            throw std::runtime_error("Failed to begin command buffer!");

        beginRenderpass(cmd);

        isFrameRunning = true;
        _info.cmd = cmd;

        return cmd;
    }

    void vk_renderer::endFrame(VkCommandBuffer cmd)
    {
        assert(isFrameRunning && "Must have started the frame before ending it!");

        endRenderpass(cmd);
        if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
            throw std::runtime_error("Failed to end command buffer!");

        swapchain->submitCommandBuffers(&cmd, &imageIndex);

        _info.cmd = VK_NULL_HANDLE;
        isFrameRunning = false;
    }

    void vk_renderer::renderScene()
    {
        assert(isFrameRunning && "Must have started the frame before rendering!");
        VkCommandBuffer cmd = currentCommandBuffer();

        pipeline->bind(cmd);

        _info.scene->for_all<eng::model_t, eng::transform_t>([&](ecs::entity_id_t id, eng::model_t& model, eng::transform_t& transform) 
        {
            transform.applyRotation(glm::vec3(0.0f, 0.5f, 0.6f)); 

            pcModelMatrix modelMatrix = { transform.mat4() };

            vkCmdPushConstants(
                cmd,
                pipeline->layout(),
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(modelMatrix),
                &modelMatrix
            );

            model.bind(cmd);
            model.draw(cmd);
        });
    }
} // namespace vk
