#include "vk_application.hpp"
#include "core/ecs.hpp"
#include <iostream>

#include "engine/camera_t.hpp"
#include <chrono>

namespace vk
{
    vk_application::vk_application()
    {
        constexpr uint32_t width = 900;
        constexpr uint32_t height = 700;
        const char* title = "Vulkan Engine";

        window = std::make_unique<vk_window>(width, height, title);

        vkContextCreateInfo info{};
        info.pWindow = window.get();

        context.init(info);

        device = std::make_unique<vk_device>(context);
        swapchain = std::make_unique<vk_swapchain>(device, context);

        const std::string pathToVertex = "src\\shaders\\test.vert.spv";
        const std::string pathToFragment = "src\\shaders\\test.frag.spv";

        pipelineCreateInfo pipelineInfo{};
        vk_pipeline::defaultPipelineCreateInfo(pipelineInfo);
        pipelineInfo.descriptorSetLayouts = device->getSetLayouts();

        pipeline = std::make_unique<vk_pipeline>(device, swapchain, pathToVertex, pathToFragment, pipelineInfo);
        renderer = std::make_unique<vk_renderer>(pipeline, swapchain, device, context, window);

        core::input::setWindow(window->window());
    }

    vk_application::~vk_application()
    {
    }

    void handleCamInput(eng::transform_t& camtransform)
    {
        static float camSpeed = 200.0f;
        float deltaTime = vk_renderer::dt(); // Store dt once

        glm::vec3 movement(0.0f);
        glm::vec3 rotation(0.0f);

        if (core::input::isKey(core::input::key::KEY_W, core::input::key_action::ACTION_PRESS))
            movement -= camtransform.forward() * camSpeed;

        if (core::input::isKey(core::input::key::KEY_S, core::input::key_action::ACTION_PRESS))
            movement += camtransform.forward() * camSpeed;

        if (core::input::isKey(core::input::key::KEY_A, core::input::key_action::ACTION_PRESS))
            movement += camtransform.left() * camSpeed;

        if (core::input::isKey(core::input::key::KEY_D, core::input::key_action::ACTION_PRESS))
            movement -= camtransform.left() * camSpeed;

        if (core::input::isKey(core::input::key::KEY_E, core::input::key_action::ACTION_PRESS))
            movement -= camtransform.up() * camSpeed;

        if (core::input::isKey(core::input::key::KEY_Q, core::input::key_action::ACTION_PRESS))
            movement += camtransform.up() * camSpeed;

        if (core::input::isKey(core::input::key::KEY_LEFT, core::input::key_action::ACTION_PRESS))
            rotation += glm::vec3(0.0f, camSpeed * 20.0f, 0.0f);

        if (core::input::isKey(core::input::key::KEY_RIGHT, core::input::key_action::ACTION_PRESS))
            rotation += glm::vec3(0.0f, -camSpeed * 20.0f, 0.0f);

        camtransform.translation += movement * deltaTime;
        camtransform.applyRotation(rotation * deltaTime);
    }
    
    void vk_application::run()
    {
        ecs::scene_t<> scene;

        struct globalUbo
        {
            alignas(16) glm::mat4 projection{1.0f};
            alignas(16) glm::mat4 view{1.0f};
        } globalUbo;

        eng::camera_t cam{scene};

        vk::vk_buffer globalBuffer(
            device,
            &globalUbo,
            sizeof(globalUbo),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VMA_MEMORY_USAGE_CPU_TO_GPU
        );

        VkDescriptorBufferInfo globalInfo{};
        globalInfo.buffer = globalBuffer.buffer();
        globalInfo.offset = 0;
        globalInfo.range = sizeof(globalBuffer);

        vk_descriptordata camData{};
        camData.pBufferInfo = &globalInfo;

        std::pair<uint32_t, uint32_t> cameraChannelInfo = device->setDescriptorData(camData);
        renderer->setScene(scene);

        eng::model_t model;
        eng::modelloader_t::loadModel("src/resource/suzane.obj", &model, device);

        ecs::entity_id_t modelId = scene.create();
        scene.construct<eng::transform_t>(modelId).translation = {0.0f, 0.0f, 0.0f};
        scene.construct<eng::model_t>(modelId) = model;

        while (!window->should_close())
        {
            glfwPollEvents();

            if (VkCommandBuffer cmd = renderer->startFrame()) 
            {
                std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

                cam.perspective(70.f, renderer->aspectRatio());

                eng::transform_t& camTransform = scene.get<eng::transform_t>(cam.getId());
                handleCamInput(camTransform);
    
                globalUbo.projection = cam.getProjection();
                globalUbo.view = cam.getView();
                
                globalBuffer.update(&globalUbo);
                globalBuffer.bindUniform(cmd, pipeline->layout(), device, cameraChannelInfo);

                renderer->renderScene();
                renderer->endFrame(cmd);

                std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
                renderer->getFrameInfo().deltaTime = std::chrono::duration<double>(end - start).count();
            }
        }

        vkDeviceWaitIdle(device->device());
    }
} // namespace vk
