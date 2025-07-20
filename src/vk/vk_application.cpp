#include "vk_application.hpp"
#include <iostream>

namespace vk
{
    vk_application::vk_application()
    {
        constexpr uint32_t width = 700;
        constexpr uint32_t height = 500;
        const char* title = "Vulkan Engine";

        window = std::make_unique<vk_window>(width, height, title);

        vkContextCreateInfo info{};
        info.pWindow = window.get();

        context.init(info);

        device = std::make_unique<vk_device>(context);
        swapchain = std::make_unique<vk_swapchain>(device, context);

        const std::string pathToVertex = "C:\\Users\\gabri\\OneDrive\\Documentos\\GitHub\\VulkanSetup\\src\\shaders\\test.vert.spv";
        const std::string pathToFragment = "C:\\Users\\gabri\\OneDrive\\Documentos\\GitHub\\VulkanSetup\\src\\shaders\\test.frag.spv";

        pipelineCreateInfo pipelineInfo{};
        vk_pipeline::defaultPipelineCreateInfo(pipelineInfo);

        pipeline = std::make_unique<vk_pipeline>(device, pathToVertex, pathToFragment, pipelineInfo);

        createCommandBuffers();
    }

    vk_application::~vk_application()
    {
        vkFreeCommandBuffers(device->device(), swapchain->commandPool(), commandBuffers.size(), commandBuffers.data());
        commandBuffers.clear();
    }

    void vk_application::createCommandBuffers()
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

    void vk_application::beginRenderpass(VkCommandBuffer cmd)
    {        
        VkRenderingAttachmentInfoKHR colorAttachment{};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        colorAttachment.imageView = swapchain->imageView(imageIndex);
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        VkClearValue clearColor = {{{0.01f, 0.0f, 0.0f, 1.0f}}};
        colorAttachment.clearValue = clearColor;

        VkRenderingAttachmentInfoKHR depthAttachment{};
        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        depthAttachment.imageView = swapchain->depthImageView(imageIndex);
        depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        VkClearValue depthClear = {{1.0f, 0}};
        depthAttachment.clearValue = depthClear;

        VkRenderingInfoKHR renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
        renderingInfo.renderArea.offset = {0, 0};
        renderingInfo.renderArea.extent = swapchain->extent();
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;
        renderingInfo.pDepthAttachment = &depthAttachment;

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
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        vkCmdBeginRenderingKHR(cmd, &renderingInfo);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(renderingInfo.renderArea.extent.width);
        viewport.height = static_cast<float>(renderingInfo.renderArea.extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{{0, 0}, swapchain->extent()};
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);
    }

    void vk_application::endRenderpass(VkCommandBuffer cmd)
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

    void vk_application::run()
    {
        while (!window->should_close())
        {
            glfwPollEvents();

            VkResult result = swapchain->acquireNextImage(&imageIndex);
            
            VkCommandBuffer cmd = commandBuffers[imageIndex];
            vkResetCommandBuffer(cmd, 0);

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS)
                throw std::runtime_error("Failed to begin command buffer!");

            beginRenderpass(cmd);
            pipeline->bind(cmd);

            vkCmdDraw(cmd, 3, 1, 0, 0);

            endRenderpass(cmd);

            if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
                throw std::runtime_error("Failed to end command buffer!");

            swapchain->submitCommandBuffers(&cmd, &imageIndex);
        }

        vkDeviceWaitIdle(device->device());
    }
} // namespace vk
