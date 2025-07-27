#pragma once

#include <volk/volk.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "transform_t.hpp"

#include <vma/vk_mem_alloc.h>
#include "vk/vk_buffer.hpp"

#include <array>

namespace eng
{
    class model_t
    {
    public:
        struct vertex_t
        {
            glm::vec3 translation;
            glm::vec3 color;
            glm::vec3 normal;
            glm::vec2 uv;

            static VkVertexInputBindingDescription getBindingDescription();
            static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions();
        };

        model_t(std::vector<vertex_t>& vertices, std::vector<uint32_t>& indices, std::unique_ptr<vk::vk_device>& device);
        ~model_t();

        void bind(VkCommandBuffer cmd);
        void draw(VkCommandBuffer cmd);
    private:
        void createVertexBuffer(std::unique_ptr<vk::vk_device>& device);
        void createIndexBuffer(std::unique_ptr<vk::vk_device>& device);

        std::vector<vertex_t> vertices;
        std::vector<uint32_t> indices;

        std::shared_ptr<vk::vk_buffer> _vertexBuffer;
        std::shared_ptr<vk::vk_buffer> _indexBuffer;
    };

} // namespace eng
