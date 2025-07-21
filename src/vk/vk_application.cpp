#include "vk_application.hpp"
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

    glm::vec3 midpoint(const glm::vec3& a, const glm::vec3& b) {
        return 0.5f * (a + b);
    }

    void generateSierpinski(int depth,
                        glm::vec3 a,
                        glm::vec3 b,
                        glm::vec3 c,
                        std::vector<eng::model_t::vertex_t>& vertices,
                        std::vector<uint32_t>& indices)
    {
        if (depth == 0) {
            uint32_t baseIndex = static_cast<uint32_t>(vertices.size());

            vertices.push_back({ a, glm::vec3(0, 0, 1), glm::vec2(0.5f, 0.0f) });
            vertices.push_back({ b, glm::vec3(0, 0, 1), glm::vec2(1.0f, 1.0f) });
            vertices.push_back({ c, glm::vec3(0, 0, 1), glm::vec2(0.0f, 1.0f) });

            indices.push_back(baseIndex);
            indices.push_back(baseIndex + 1);
            indices.push_back(baseIndex + 2);
            return;
        }

        glm::vec3 ab = midpoint(a, b);
        glm::vec3 bc = midpoint(b, c);
        glm::vec3 ca = midpoint(c, a);

        generateSierpinski(depth - 1, a, ab, ca, vertices, indices);
        generateSierpinski(depth - 1, ab, b, bc, vertices, indices);
        generateSierpinski(depth - 1, ca, bc, c, vertices, indices);
    }

    void vk_application::run()
    {
        std::vector<eng::model_t::vertex_t> triangle_vertices;
        std::vector<uint32_t> triangle_indices;

        generateSierpinski(4,                            // depth of recursion
            glm::vec3(0.0f, -0.5f, 0.0f),                // bottom
            glm::vec3(0.5f,  0.5f, 0.0f),                // right
            glm::vec3(-0.5f, 0.5f, 0.0f),                // left
            triangle_vertices, triangle_indices);

        eng::model_t sierpinski(triangle_vertices, triangle_indices, device);

        while (!window->should_close())
        {
            glfwPollEvents();

            if (VkCommandBuffer cmd = renderer->startFrame()) 
            {
                pipeline->bind(cmd);

                sierpinski.bind(cmd);
                sierpinski.draw(cmd);

                renderer->endFrame(cmd);
            }
        }

        vkDeviceWaitIdle(device->device());
    }
} // namespace vk
