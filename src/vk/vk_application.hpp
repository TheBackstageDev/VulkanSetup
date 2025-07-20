#pragma once

#include "vk_window.hpp"
#include "vk_context.hpp"
#include "vk_device.hpp"
#include "vk_swapchain.hpp"
#include "vk_pipeline.hpp"

#include <memory>

namespace vk
{
    class vk_application
    {
    public:
        vk_application();
        ~vk_application();

        void run();

    private:
        vk_context context;

        void createCommandBuffers();
        void recordCommandBuffers();

        void beginRenderpass(VkCommandBuffer cmd);
        void endRenderpass(VkCommandBuffer cmd);

        std::unique_ptr<vk_window> window;
        std::unique_ptr<vk_device> device;
        std::unique_ptr<vk_swapchain> swapchain;
        std::unique_ptr<vk_pipeline> pipeline;

        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t imageIndex = 0;
    };
}