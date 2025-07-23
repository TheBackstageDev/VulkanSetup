#pragma once

#include <volk/volk.h>
#include "vk_swapchain.hpp"
#include "vk_pipeline.hpp"

#include "engine/model_t.hpp"
#include "core/ecs.hpp"

namespace vk
{
    struct pcModelMatrix
    {
        glm::mat4 modelMatrix;
    };

    class vk_renderer
    {
    public:
        vk_renderer(std::unique_ptr<vk_pipeline>& pipeline, std::shared_ptr<vk_swapchain>& swapchain, std::unique_ptr<vk_device>& device, vk_context& context, std::unique_ptr<vk_window>& window);
        ~vk_renderer();

        VkCommandBuffer startFrame();
        void endFrame(VkCommandBuffer cmd);

        void renderScene(ecs::scene_t<>& scene);

    private:
        std::shared_ptr<vk_swapchain>& swapchain;
        std::unique_ptr<vk_device>& device;
        std::unique_ptr<vk_window>& window;
        std::unique_ptr<vk_pipeline>& pipeline;
        vk_context& context;

        VkCommandBuffer currentCommandBuffer() { return commandBuffers[imageIndex]; }
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapchain();

        void beginRenderpass(VkCommandBuffer cmd);
        void endRenderpass(VkCommandBuffer cmd);

        std::vector<VkCommandBuffer> commandBuffers;
        uint32_t imageIndex = 0;

        bool isFrameRunning = false;
    };
    
} // namespace vk
