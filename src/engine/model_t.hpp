#pragma once

#include <volk/volk.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

#include <vma/vk_mem_alloc.h>
#include "vk/vk_buffer.hpp"

#include <array>

namespace eng
{
    struct transform_t
    {
        glm::vec3 translation{0.0f};
        glm::vec3 scale{0.5f};
        glm::quat rotation;

        void applyRotation(glm::vec3 newRotation)
        {
            glm::vec3 axis = newRotation - getRotationEuler();
            float angle = glm::length(axis);

            if (angle > glm::epsilon<float>())
            {
                axis = glm::normalize(axis);
                glm::quat deltaRotation = glm::angleAxis(glm::radians(angle), glm::normalize(axis));

                rotation = glm::normalize(deltaRotation * rotation); // Normalize the quaternion to avoid drift
            }
        }

        void rotateEuler(glm::vec3 angles)
        {
            rotation = glm::normalize(glm::quat(glm::radians(angles)));
        }

        glm::vec3 getRotationEuler() { return glm::degrees(glm::eulerAngles(rotation)); }

        glm::mat4 mat4();
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
