#pragma once

#include <volk/volk.h>
#include "vk_swapchain.hpp"

namespace vk
{
    class vk_renderer
    {
    public:
        vk_renderer(std::shared_ptr<vk_swapchain>& swapchain, std::unique_ptr<vk_device>& device, vk_context& context, std::unique_ptr<vk_window>& window);
        ~vk_renderer();

        VkCommandBuffer startFrame();
        void endFrame(VkCommandBuffer cmd);

    private:
        std::shared_ptr<vk_swapchain>& swapchain;
        std::unique_ptr<vk_device>& device;
        std::unique_ptr<vk_window>& window;
        vk_context& context;

        VkCommandBuffer currentCommandBuffer() { return commandBuffers[imageIndex]; }
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapchain();

        void beginRenderpass(VkCommandBuffer cmd);
        void endRenderpass(VkCommandBuffer cmd);

        std::vector<VkCommandBuffer> commandBuffers;
        uint32_t imageIndex = 0;
    };
    
} // namespace vk
