#pragma once

#include <volk/volk.h>
#include "vk_swapchain.hpp"
#include "vk_pipeline.hpp"

#include "engine/model_t.hpp"
#include "core/ecs.hpp"

namespace vk
{
    struct pcPush
    {
        alignas(16) glm::mat4 modelMatrix;
        alignas(16) uint32_t textureId = 0;
    };

    struct frameinfo_t
    {
        VkCommandBuffer cmd = VK_NULL_HANDLE;
        ecs::scene_t<>* scene = nullptr;

        float deltaTime = 0.0f;
    };

    class vk_renderer
    {
    public:
        vk_renderer(std::unique_ptr<vk_pipeline>& pipeline, std::shared_ptr<vk_swapchain>& swapchain, std::unique_ptr<vk_device>& device, vk_context& context, std::unique_ptr<vk_window>& window);
        ~vk_renderer();

        VkCommandBuffer startFrame();
        void endFrame(VkCommandBuffer cmd);

        void setScene(ecs::scene_t<>& scene) { _info.scene = &scene; }
        void renderScene();
        
        float aspectRatio() { return swapchain->getAspectRatio(); }

        static frameinfo_t& getFrameInfo() { return _info; }
        static float dt() { return _info.deltaTime; }
    private:
        std::shared_ptr<vk_swapchain>& swapchain;
        std::unique_ptr<vk_device>& device;
        std::unique_ptr<vk_window>& window;
        std::unique_ptr<vk_pipeline>& pipeline;
        vk_context& context;

        static frameinfo_t _info;

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
