#pragma once

#include <volk/volk.h>
#include "vk_device.hpp"
#include "vk_swapchain.hpp"

#include <string>
#include <vector>

namespace vk
{
    struct pipelineCreateInfo
    {
        pipelineCreateInfo() = default;
        pipelineCreateInfo(const pipelineCreateInfo &) = delete;
        pipelineCreateInfo &operator=(const pipelineCreateInfo &) = delete;

        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        VkPipelineRenderingCreateInfo pipelineRenderingInfo;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;
        VkPipelineLayout pipelineLayout = nullptr;

        VkFormat colorFormat = VK_FORMAT_R8G8B8A8_SRGB;
        VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
    };

    class vk_pipeline
    {
    public:
        vk_pipeline(
            std::unique_ptr<vk_device>& device,
            std::unique_ptr<vk_swapchain>& swapchain,
            const std::string &vertFilepath,
            const std::string &fragFilepath,
            const pipelineCreateInfo &createInfo);
            
        ~vk_pipeline();

        void bind(VkCommandBuffer commandBuffer) { vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline); }
        static void defaultPipelineCreateInfo(pipelineCreateInfo &createInfo);

        vk_pipeline(const vk_pipeline &) = delete;
        vk_pipeline operator=(const vk_pipeline &) = delete;

        VkPipeline pipeline() const { return _pipeline; }
        VkPipelineLayout layout() const { return _pipelineLayout; }
    private:
        static std::vector<char> readFile(const std::string &filepath);
        void createShaderModule(const std::vector<char>& code, VkShaderModule& module);

        void createPipeline(const std::string &vertFilepath,
            const std::string &fragFilepath,
            const pipelineCreateInfo &createInfo);

        VkPipeline _pipeline = VK_NULL_HANDLE;
        VkPipelineLayout _pipelineLayout = VK_NULL_HANDLE;

        std::unique_ptr<vk_device>& device;
        std::unique_ptr<vk_swapchain>& swapchain;
    };
    
} // namespace vk
