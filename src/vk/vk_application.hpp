#pragma once

#include "vk_window.hpp"
#include "vk_context.hpp"
#include "vk_device.hpp"
#include "vk_swapchain.hpp"
#include "vk_pipeline.hpp"
#include "vk_renderer.hpp"

#include "engine/modelloader_t.hpp"
#include "core/input.hpp"

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

        std::unique_ptr<vk_window> window;
        std::unique_ptr<vk_device> device;
        std::shared_ptr<vk_swapchain> swapchain;
        std::unique_ptr<vk_pipeline> pipeline;
        std::unique_ptr<vk_renderer> renderer;
    };
}