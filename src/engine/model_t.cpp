#include "model_t.hpp"
#include "vk/vk_context.hpp"

namespace eng
{
    // Vertex
    
    VkVertexInputBindingDescription model_t::vertex_t::getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.stride = sizeof(vertex_t);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        bindingDescription.binding = 0;

        return bindingDescription;
    }

    std::array<VkVertexInputAttributeDescription, 4> model_t::vertex_t::getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions;

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].offset = offsetof(vertex_t, vertex_t::translation);
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].offset = offsetof(vertex_t, vertex_t::color);
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].offset = offsetof(vertex_t, vertex_t::normal);
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        
        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].offset = offsetof(vertex_t, vertex_t::uv);
        attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
        
        return attributeDescriptions;
    }

    // Transform

    glm::mat4 transform_t::mat4()
    {
        glm::mat4 matrix(1.0f);

        glm::mat4 scaleMatrix = glm::scale(matrix, scale);
        glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
        glm::mat4 translationMatrix = glm::translate(matrix, translation);

        return translationMatrix * rotationMatrix * scaleMatrix;
    }

    // Model

    model_t::model_t(std::vector<vertex_t>& vertices, std::vector<uint32_t>& indices, std::unique_ptr<vk::vk_device>& device)
        : vertices(vertices), indices(indices)
    {
        createVertexBuffer(device);
        createIndexBuffer(device);
    }

    model_t::~model_t()
    {
    }

    void model_t::bind(VkCommandBuffer cmd)
    {
        VkBuffer vertexBuffers[] = { _vertexBuffer->buffer() };
        VkDeviceSize offsets[] = { 0 };

        vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(cmd, _indexBuffer->buffer(), 0, VK_INDEX_TYPE_UINT32);
    }

    void model_t::draw(VkCommandBuffer cmd)
    {
        vkCmdDrawIndexed(cmd, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
    }

    void model_t::createVertexBuffer(std::unique_ptr<vk::vk_device>& device)
    {
        VkDeviceSize bufferSize = sizeof(vertex_t) * vertices.size();

        _vertexBuffer = std::make_unique<vk::vk_buffer>(device,
                                    vertices.data(),
                                    bufferSize,
                                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                    VMA_MEMORY_USAGE_CPU_TO_GPU);
    }

    void model_t::createIndexBuffer(std::unique_ptr<vk::vk_device>& device)
    {
        VkDeviceSize bufferSize = sizeof(uint32_t) * indices.size();
        
        _indexBuffer = std::make_unique<vk::vk_buffer>(device,
                                    indices.data(),
                                    bufferSize,
                                    VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                    VMA_MEMORY_USAGE_CPU_TO_GPU);
    }
    
} // namespace eng
