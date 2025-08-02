#pragma once

#include <volk/volk.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "transform_t.hpp"

#include <vma/vk_mem_alloc.h>
#include "vk/vk_buffer.hpp"

#include <array>
#include "core/imageloader.hpp"

namespace eng
{
    struct texture_t
    {
        uint32_t id = UINT32_MAX;
        uint32_t channelId = UINT32_MAX;
    };

    class model_t
    {
    public:
        struct vertex_t
        {
            glm::vec3 translation;
            glm::vec3 color;
            glm::vec3 normal;
            glm::vec2 uv;
            glm::vec3 tangent;

            static VkVertexInputBindingDescription getBindingDescription();
            static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions();
        };

        using index_t = uint32_t;

        model_t() = default;
        model_t(std::vector<vertex_t>& vertices, std::vector<index_t>& indices);
        ~model_t();

        void bind(VkCommandBuffer cmd);
        void draw(VkCommandBuffer cmd);
    private:
        void createVertexBuffer();
        void createIndexBuffer();

        std::vector<vertex_t> vertices;
        std::vector<index_t> indices;

        std::shared_ptr<vk::vk_buffer> _vertexBuffer;
        std::shared_ptr<vk::vk_buffer> _indexBuffer;
    };

} // namespace eng
