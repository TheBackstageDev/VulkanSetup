#include "vk_renderer.hpp"
#include <iostream>

namespace vk
{
    vk_renderer::vk_renderer(std::shared_ptr<vk_swapchain>& swapchain, std::unique_ptr<vk_device>& device, vk_context& context, std::unique_ptr<vk_window>& window)
        : swapchain(swapchain), device(device), context(context), window(window)
    {
        createCommandBuffers();
    }

    vk_renderer::~vk_renderer()
    {
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
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};      // Clear color
        clearValues[1].depthStencil = {1.0f, 0};                // Clear depth

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = swapchain->renderPass();
        renderPassInfo.framebuffer = swapchain->framebuffer(imageIndex);
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapchain->extent();
        renderPassInfo.clearValueCount = 2;
        renderPassInfo.pClearValues = clearValues;

        vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(renderPassInfo.renderArea.extent.width);
        viewport.height = static_cast<float>(renderPassInfo.renderArea.extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{{0, 0}, swapchain->extent()};
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);
    }

    void vk_renderer::endRenderpass(VkCommandBuffer cmd)
    {
        vkCmdEndRenderPass(cmd);
    }

    VkCommandBuffer vk_renderer::startFrame()
    {
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

        return cmd;
    }

    void vk_renderer::endFrame(VkCommandBuffer cmd)
    {
        endRenderpass(cmd);
        if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
            throw std::runtime_error("Failed to end command buffer!");

        swapchain->submitCommandBuffers(&cmd, &imageIndex);
    }
} // namespace vk
