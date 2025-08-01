#pragma once

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/backends/imgui_impl_glfw.h>

#include "vk_window.hpp"
#include "vk_context.hpp"
#include "vk_device.hpp"
#include "vk_swapchain.hpp"
#include "vk_pipeline.hpp"
#include "vk_renderer.hpp"

#include "engine/modelloader_t.hpp"
#include "core/input.hpp"
#include "core/imageloader.hpp"

#include <memory>

struct globalUbo
{
    alignas(16) glm::mat4 projection{1.0f};
    alignas(16) glm::mat4 view{1.0f};
};

namespace vk
{
    class vk_application
    {
    public:
        vk_application();
        ~vk_application();

        void run();
    private:
        void initImgui(VkPipelineRenderingCreateInfo& pipelineRenderingInfo);
        void setupBuffers();

        VkDescriptorPool imguiPool;

        vk_context context;

        std::unique_ptr<vk_buffer> globalBuffer;

        // channel infos
        std::pair<uint32_t, uint32_t> globaluboChannelInfo;
        std::pair<uint32_t, uint32_t> defaultTextureChannelInfo;


        std::unique_ptr<vk_window> window;
        std::unique_ptr<vk_device> device;
        std::shared_ptr<vk_swapchain> swapchain;
        std::unique_ptr<vk_pipeline> pipeline;
        std::unique_ptr<vk_renderer> renderer;
    };
}