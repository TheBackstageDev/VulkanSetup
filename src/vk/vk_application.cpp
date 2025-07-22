#include "vk_application.hpp"
#include "core/ecs.hpp"
#include <iostream>

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

        const std::string pathToVertex = "C:\\Users\\gabri\\OneDrive\\Documentos\\GitHub\\VulkanSetup\\src\\shaders\\test.vert.spv";
        const std::string pathToFragment = "C:\\Users\\gabri\\OneDrive\\Documentos\\GitHub\\VulkanSetup\\src\\shaders\\test.frag.spv";

        pipelineCreateInfo pipelineInfo{};
        vk_pipeline::defaultPipelineCreateInfo(pipelineInfo);

        renderer = std::make_unique<vk_renderer>(swapchain, device, context, window);
        pipeline = std::make_unique<vk_pipeline>(device, swapchain, pathToVertex, pathToFragment, pipelineInfo);
    }

    vk_application::~vk_application()
    {
    }

    struct pcModelMatrix
    {
        glm::mat4 modelMatrix;
    };

    void vk_application::run()
    {
        std::vector<eng::model_t::vertex_t> cube_vertices = {
            // Front face – Blue (z = 0.5)
            { {-0.5f, -0.5f,  0.5f}, {0, 0, 1}, {0, 0, 1}, {0, 0} }, // Vertex 0: Bottom-left
            { { 0.5f, -0.5f,  0.5f}, {0, 0, 1}, {0, 0, 1}, {1, 0} }, // Vertex 1: Bottom-right
            { { 0.5f,  0.5f,  0.5f}, {0, 0, 1}, {0, 0, 1}, {1, 1} }, // Vertex 2: Top-right
            { {-0.5f,  0.5f,  0.5f}, {0, 0, 1}, {0, 0, 1}, {0, 1} }, // Vertex 3: Top-left

            // Back face – Red (z = -0.5)
            { {-0.5f, -0.5f, -0.5f}, {1, 0, 0}, {0, 0, -1}, {1, 0} }, // Vertex 4: Bottom-right
            { { 0.5f, -0.5f, -0.5f}, {1, 0, 0}, {0, 0, -1}, {0, 0} }, // Vertex 5: Bottom-left
            { { 0.5f,  0.5f, -0.5f}, {1, 0, 0}, {0, 0, -1}, {0, 1} }, // Vertex 6: Top-left
            { {-0.5f,  0.5f, -0.5f}, {1, 0, 0}, {0, 0, -1}, {1, 1} }, // Vertex 7: Top-right

            // Left face – Green (x = -0.5)
            { {-0.5f, -0.5f, -0.5f}, {0, 1, 0}, {-1, 0, 0}, {0, 0} }, // Vertex 8: Bottom-back
            { {-0.5f, -0.5f,  0.5f}, {0, 1, 0}, {-1, 0, 0}, {1, 0} }, // Vertex 9: Bottom-front
            { {-0.5f,  0.5f,  0.5f}, {0, 1, 0}, {-1, 0, 0}, {1, 1} }, // Vertex 10: Top-front
            { {-0.5f,  0.5f, -0.5f}, {0, 1, 0}, {-1, 0, 0}, {0, 1} }, // Vertex 11: Top-back

            // Right face – Yellow (x = 0.5)
            { { 0.5f, -0.5f,  0.5f}, {1, 1, 0}, {1, 0, 0}, {0, 0} }, // Vertex 12: Bottom-front
            { { 0.5f, -0.5f, -0.5f}, {1, 1, 0}, {1, 0, 0}, {1, 0} }, // Vertex 13: Bottom-back
            { { 0.5f,  0.5f, -0.5f}, {1, 1, 0}, {1, 0, 0}, {1, 1} }, // Vertex 14: Top-back
            { { 0.5f,  0.5f,  0.5f}, {1, 1, 0}, {1, 0, 0}, {0, 1} }, // Vertex 15: Top-front

            // Top face – Cyan (y = 0.5)
            { {-0.5f,  0.5f,  0.5f}, {0, 1, 1}, {0, 1, 0}, {0, 0} }, // Vertex 16: Front-left
            { { 0.5f,  0.5f,  0.5f}, {0, 1, 1}, {0, 1, 0}, {1, 0} }, // Vertex 17: Front-right
            { { 0.5f,  0.5f, -0.5f}, {0, 1, 1}, {0, 1, 0}, {1, 1} }, // Vertex 18: Back-right
            { {-0.5f,  0.5f, -0.5f}, {0, 1, 1}, {0, 1, 0}, {0, 1} }, // Vertex 19: Back-left

            // Bottom face – Magenta (y = -0.5)
            { {-0.5f, -0.5f, -0.5f}, {1, 0, 1}, {0, -1, 0}, {0, 0} }, // Vertex 20: Back-left
            { { 0.5f, -0.5f, -0.5f}, {1, 0, 1}, {0, -1, 0}, {1, 0} }, // Vertex 21: Back-right
            { { 0.5f, -0.5f,  0.5f}, {1, 0, 1}, {0, -1, 0}, {1, 1} }, // Vertex 22: Front-right
            { {-0.5f, -0.5f,  0.5f}, {1, 0, 1}, {0, -1, 0}, {0, 1} }  // Vertex 23: Front-left
        };

        std::vector<uint32_t> cube_indices = {
            // Front face (0–3)
            0, 1, 2, 0, 2, 3,  // Triangles: (0,1,2), (0,2,3)
            // Back face (4–7)
            4, 6, 5, 4, 7, 6,  // Triangles: (4,6,5), (4,7,6)
            // Left face (8–11)
            8, 9, 10, 8, 10, 11,  // Triangles: (8,9,10), (8,10,11)
            // Right face (12–15)
            12, 14, 13, 12, 15, 14,  // Triangles: (12,14,13), (12,15,14)
            // Top face (16–19)
            16, 18, 17, 16, 19, 18,  // Triangles: (16,18,17), (16,19,18)
            // Bottom face (20–23)
            20, 21, 22, 20, 22, 23  // Triangles: (20,21,22), (20,22,23)
        };
        
        eng::model_t cube(cube_vertices, cube_indices, device);

        ecs::scene_t<> scene;

        ecs::entity_id_t id = scene.create();
        scene.construct<eng::model_t>(id, cube_vertices, cube_indices, device);
        scene.construct<eng::transform_t>(id);

        glm::vec3 rotation{0.0f, 0.0f, 0.0f};

        while (!window->should_close())
        {
            glfwPollEvents();

            if (VkCommandBuffer cmd = renderer->startFrame()) 
            {
                pipeline->bind(cmd);

                eng::transform_t& transform = scene.get<eng::transform_t>(id); 
                eng::model_t& model = scene.get<eng::model_t>(id);

                rotation.y += 1.0f;
                rotation.x += 0.1f;

                transform.rotateEuler(rotation);
                pcModelMatrix modelMatrix { transform.mat4() };

                vkCmdPushConstants(
                    cmd,
                    pipeline->layout(),
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    0,
                    sizeof(modelMatrix),
                    &modelMatrix
                );

                model.bind(cmd);
                model.draw(cmd);

                renderer->endFrame(cmd);
            }
        }

        vkDeviceWaitIdle(device->device());
    }
} // namespace vk
