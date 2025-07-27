#include "vk_application.hpp"
#include "core/ecs.hpp"
#include <iostream>

#include "engine/camera_t.hpp"

namespace vk
{
    vk_application::vk_application()
    {
        constexpr uint32_t width = 700;
        constexpr uint32_t height = 500;
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
    }

    vk_application::~vk_application()
    {
    }
    
    void vk_application::run()
    {
        std::vector<eng::model_t::vertex_t> cube_vertices = {
            // Front face (z = 0.5) – Blue
            { {-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f} }, // Vertex 0: Bottom-left
            { { 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f} }, // Vertex 1: Bottom-right
            { { 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f} }, // Vertex 2: Top-right
            { {-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f} }, // Vertex 3: Top-left

            // Back face (z = -0.5) – Red
            { {-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f} }, // Vertex 4: Bottom-left
            { { 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f} }, // Vertex 5: Bottom-right
            { { 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f} }, // Vertex 6: Top-right
            { {-0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f} }, // Vertex 7: Top-left

            // Left face (x = -0.5) – Green
            { {-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} }, // Vertex 8: Bottom-back
            { {-0.5f, -0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} }, // Vertex 9: Bottom-front
            { {-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f} }, // Vertex 10: Top-front
            { {-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f} }, // Vertex 11: Top-back

            // Right face (x = 0.5) – Yellow
            { { 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} }, // Vertex 12: Bottom-front
            { { 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} }, // Vertex 13: Bottom-back
            { { 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f} }, // Vertex 14: Top-back
            { { 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f} }, // Vertex 15: Top-front

            // Top face (y = 0.5) – Cyan
            { {-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} }, // Vertex 16: Front-left
            { { 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f} }, // Vertex 17: Front-right
            { { 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f} }, // Vertex 18: Back-right
            { {-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f} }, // Vertex 19: Back-left

            // Bottom face (y = -0.5) – Magenta
            { {-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f} }, // Vertex 20: Back-left
            { { 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f} }, // Vertex 21: Back-right
            { { 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f} }, // Vertex 22: Front-right
            { {-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f} }  // Vertex 23: Front-left
        };

        std::vector<uint32_t> cube_indices = {
            // Front face (0–3): 0, 1, 2,  0, 2, 3 -> 0, 2, 1,  0, 3, 2
            0, 2, 1,  0, 3, 2,
            
            // Back face (4–7): 4, 7, 6,  4, 6, 5 -> 4, 6, 7,  4, 5, 6
            4, 6, 7,  4, 5, 6,
            
            // Left face (8–11): 8, 9, 10,  8, 10, 11 -> 8, 10, 9,  8, 11, 10
            8, 10, 9,  8, 11, 10,
            
            // Right face (12–15): 12, 13, 14,  12, 14, 15 -> 12, 14, 13,  12, 15, 14
            12, 14, 13,  12, 15, 14,
            
            // Top face (16–19): 16, 17, 18,  16, 18, 19 -> 16, 18, 17,  16, 19, 18
            16, 18, 17,  16, 19, 18,
            
            // Bottom face (20–23): 20, 21, 22,  20, 22, 23 -> 20, 22, 21,  20, 23, 22
            20, 22, 21,  20, 23, 22
        };

        eng::model_t cube(cube_vertices, cube_indices, device);

        ecs::scene_t<> scene;

        ecs::entity_id_t id = scene.create();
        scene.construct<eng::model_t>(id, cube_vertices, cube_indices, device);
        scene.construct<eng::transform_t>(id);
        struct globalBuffer
        {
            alignas(16) glm::mat4 projection;
            alignas(16) glm::mat4 view;
        } globalData;

        eng::transform_t& transform = scene.get<eng::transform_t>(id);
        transform.translation = {0.0f, 0.0f, 2.0f};

        eng::camera_t cam{scene};

        float aspect = swapchain->getAspectRatio();
        cam.perspective(70.f, aspect, 0.01f, 100.f);
        globalData.projection = cam.getProjection();
        globalData.view = cam.getView();

        vk::vk_buffer cameraBuffer(
            device,
            &globalData,
            sizeof(globalData),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VMA_MEMORY_USAGE_CPU_TO_GPU
        );

        VkDescriptorBufferInfo camInfo{};
        camInfo.buffer = cameraBuffer.buffer();
        camInfo.offset = 0;
        camInfo.range = sizeof(globalData);

        vk_descriptordata camData{};
        camData.pBufferInfo = &camInfo;

        std::pair<uint32_t, uint32_t> cameraChannelInfo = device->setDescriptorData(camData);

        while (!window->should_close())
        {
            glfwPollEvents();

            if (VkCommandBuffer cmd = renderer->startFrame()) 
            {
                float aspect = swapchain->getAspectRatio();

                eng::transform_t& camTransform = scene.get<eng::transform_t>(cam.getId());
                camTransform.applyRotation(glm::vec3(0.0f, 0.2f, 0.0f));

                cam.perspective(70.f, aspect, 0.01, 100);
    
                globalData.projection = cam.getProjection();
                globalData.view = cam.getView();
                cameraBuffer.update(&globalData);
                
                vkCmdBindDescriptorSets(
                    cmd,                            
                    VK_PIPELINE_BIND_POINT_GRAPHICS,   
                    pipeline->layout(),                     
                    0,                               
                    1,                          
                    &device->getDescriptorSet(cameraChannelInfo.first), 
                    0,                                 
                    &cameraChannelInfo.second         
                );

                renderer->renderScene(scene);
                renderer->endFrame(cmd);
            }
        }

        vkDeviceWaitIdle(device->device());
    }
} // namespace vk
