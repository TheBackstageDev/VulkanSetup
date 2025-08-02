#pragma once

#include <volk/volk.h>
#include "vk/vk_context.hpp"
#include "vk/vk_device.hpp"
#include "vk/vk_swapchain.hpp"
#include "vk/vk_pipeline.hpp"
#include "vk/vk_renderer.hpp"
#include "vk/vk_window.hpp"
#include "core/input.hpp"
#include "core/imageloader.hpp"

#include "core/ecs.hpp"
#include "core/systemactor.hpp"
#include "engine/transform_t.hpp"
#include "engine/camera_t.hpp"
#include "engine/modelloader_t.hpp"

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/backends/imgui_impl_glfw.h>

#include <memory>

namespace vk
{
    class vk_engine
    {
    public:
        vk_engine();
        ~vk_engine();

        void runMainLoop();
        
        template<typename T, typename... Args>
        void addActor(Args&&... args) {
            static uint32_t lastId = 0;

            auto actor = std::make_unique<T>(std::forward<Args>(args)...);
            actor->id = ++lastId;
            _actors.push_back(std::move(actor));
        }

        frameinfo_t getFrameInfo() const { return vk_renderer::getFrameInfo(); }
    private:
        struct globalUbo
        {
            alignas(16) glm::mat4 projection{1.0f};
            alignas(16) glm::mat4 view{1.0f};
        };

        void runExecutionPipeline(VkCommandBuffer cmd);
        void runRendering(VkCommandBuffer cmd);

        void initVulkan();
        void initImgui(const VkPipelineRenderingCreateInfo& pipelineRenderingInfo);
        void cleanup();

        void setupBuffers();
        void setupBaseScene();

        std::unique_ptr<vk_window> window;
        std::unique_ptr<vk_device> device;
        std::shared_ptr<vk_swapchain> swapchain;
        std::unique_ptr<vk_pipeline> pipeline;
        std::unique_ptr<vk_renderer> renderer;
        vk_context context;

        // buffers
        std::unique_ptr<vk_buffer> globalBuffer;

        // channel infos
        std::pair<uint32_t, uint32_t> globaluboChannelInfo;
        std::pair<uint32_t, uint32_t> defaultTextureChannelInfo;

        // others
        VkDescriptorPool imguiPool;

        // scene related
        ecs::scene_t<> _scene;
        eng::camera_t cam{_scene}; // Temporary

        std::vector<std::unique_ptr<core::systemactor>> _actors;
    };
} // namespace vk
