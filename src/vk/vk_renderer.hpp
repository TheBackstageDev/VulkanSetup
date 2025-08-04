#pragma once

#include <volk/volk.h>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/backends/imgui_impl_glfw.h>

#include "vk_swapchain.hpp"
#include "vk_offscreen.hpp"
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

        vk_channelinfo channelIndices;
    };

    class vk_renderer
    {
    public:
        vk_renderer(std::unique_ptr<vk_pipeline>& pipeline,  
            std::unique_ptr<vk_device>& device, vk_context& context, 
            std::unique_ptr<vk_window>& window, 
            std::shared_ptr<vk_swapchain>& swapchain,
            std::unique_ptr<vk_offscreen_renderer>* offscreen = nullptr);

        ~vk_renderer();

        VkCommandBuffer startFrame();
        void endFrame(VkCommandBuffer cmd);

        void setScene(ecs::scene_t<>& scene) { _info.scene = &scene; }
        void renderScene();
        void renderInterface();
        
        float aspectRatio() { return offscreen == nullptr ? swapchain->getAspectRatio() : offscreen->get()->aspectRatio(); }

        static frameinfo_t& getFrameInfo() { return _info; }
        static float dt() { return _info.deltaTime; }
    private:
        std::unique_ptr<vk_offscreen_renderer>* offscreen = nullptr;

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
