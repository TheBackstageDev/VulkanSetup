#include "vk_pipeline.hpp"
#include <fstream>

namespace vk
{
    vk_pipeline::vk_pipeline(
        std::unique_ptr<vk_device>& device,
        std::shared_ptr<vk_swapchain>& swapchain,
        const std::string &vertFilepath,
        const std::string &fragFilepath,
        const pipelineCreateInfo &createInfo)
        : device(device), swapchain(swapchain)
    {
        createPipeline(vertFilepath, fragFilepath, createInfo);
    }

    vk_pipeline::~vk_pipeline()
    {
        vkDestroyPipelineLayout(device->device(), _pipelineLayout, nullptr);
        vkDestroyPipeline(device->device(), _pipeline, nullptr);
    }

    std::vector<char> vk_pipeline::readFile(const std::string &filepath)
    {
        std::ifstream file{filepath, std::ios::ate | std::ios::binary};

        if (!file.is_open())
        {
            throw std::runtime_error("Failed to open file: " + filepath);
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }

    void vk_pipeline::createShaderModule(const std::vector<char>& code, VkShaderModule& module)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        if (vkCreateShaderModule(device->device(), &createInfo, nullptr, &module) != VK_SUCCESS)
            throw std::runtime_error("Failed to create shader module");
    }

    void vk_pipeline::createPipeline(
        const std::string& vertFilepath,
        const std::string& fragFilepath,
        const pipelineCreateInfo& info)
    {
        auto vertCode = readFile(vertFilepath);
        auto fragCode = readFile(fragFilepath);

        VkShaderModule vertModule, fragModule;
        createShaderModule(vertCode, vertModule);
        createShaderModule(fragCode, fragModule);

        VkPipelineShaderStageCreateInfo shaderStages[2]{};
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = vertModule;
        shaderStages[0].pName = "main";

        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = fragModule;
        shaderStages[1].pName = "main";

        VkPushConstantRange range = {};
        range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        range.offset = 0;
        range.size = 128;

        VkPipelineLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutInfo.setLayoutCount = static_cast<uint32_t>(info.descriptorSetLayouts.size());
        layoutInfo.pSetLayouts = info.descriptorSetLayouts.data();
        layoutInfo.pushConstantRangeCount = 1;
        layoutInfo.pPushConstantRanges = &range;

        auto vertexBindingDescription = eng::model_t::vertex_t::getBindingDescription();
        auto vertexAttributeDescriptions = eng::model_t::vertex_t::getAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &vertexBindingDescription;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();

        if (vkCreatePipelineLayout(device->device(), &layoutInfo, nullptr, &_pipelineLayout) != VK_SUCCESS)
            throw std::runtime_error("Failed to create pipeline layout");

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &info.inputAssemblyInfo;
        pipelineInfo.pViewportState = &info.viewportInfo;
        pipelineInfo.pRasterizationState = &info.rasterizationInfo;
        pipelineInfo.pMultisampleState = &info.multisampleInfo;
        pipelineInfo.pDepthStencilState = &info.depthStencilInfo;
        pipelineInfo.pColorBlendState = &info.colorBlendInfo;
        pipelineInfo.pDynamicState = &info.dynamicStateInfo;
        pipelineInfo.layout = _pipelineLayout;
        pipelineInfo.renderPass = nullptr;
        pipelineInfo.subpass = 0;
        pipelineInfo.pNext = &info.pipelineRenderingInfo;

        if (vkCreateGraphicsPipelines(device->device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_pipeline) != VK_SUCCESS)
            throw std::runtime_error("Failed to create graphics pipeline");

        vkDestroyShaderModule(device->device(), vertModule, nullptr);
        vkDestroyShaderModule(device->device(), fragModule, nullptr);
    }

    void vk_pipeline::defaultPipelineCreateInfo(pipelineCreateInfo& createInfo)
    {
        createInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        createInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        createInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

        createInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        createInfo.viewportInfo.viewportCount = 1;
        createInfo.viewportInfo.pViewports = nullptr;
        createInfo.viewportInfo.scissorCount = 1;
        createInfo.viewportInfo.pScissors = nullptr;

        createInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        createInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
        createInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        createInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        createInfo.rasterizationInfo.lineWidth = 1.0f;
        createInfo.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        createInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        createInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
        createInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f; // Optional
        createInfo.rasterizationInfo.depthBiasClamp = 0.0f;          // Optional
        createInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;    // Optional

        createInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        createInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
        createInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        createInfo.multisampleInfo.minSampleShading = 1.0f;          // Optional
        createInfo.multisampleInfo.pSampleMask = nullptr;            // Optional
        createInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE; // Optional
        createInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;      // Optional

        createInfo.colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        createInfo.colorBlendAttachment.blendEnable = VK_FALSE;
        createInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; 
        createInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        createInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;            
        createInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; 
        createInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        createInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;            

        createInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        createInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
        createInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
        createInfo.colorBlendInfo.attachmentCount = 1;
        createInfo.colorBlendInfo.pAttachments = &createInfo.colorBlendAttachment;
        createInfo.colorBlendInfo.blendConstants[0] = 0.0f; // Optional
        createInfo.colorBlendInfo.blendConstants[1] = 0.0f; // Optional
        createInfo.colorBlendInfo.blendConstants[2] = 0.0f; // Optional
        createInfo.colorBlendInfo.blendConstants[3] = 0.0f; // Optional

        createInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        createInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
        createInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
        createInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        createInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        createInfo.depthStencilInfo.minDepthBounds = 0.0f; // Optionalhy
        createInfo.depthStencilInfo.maxDepthBounds = 1.0f; // Optional
        createInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
        createInfo.depthStencilInfo.front = {}; // Optional
        createInfo.depthStencilInfo.back = {};  // Optional

        createInfo.pipelineRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        createInfo.pipelineRenderingInfo.colorAttachmentCount = 1;
        createInfo.pipelineRenderingInfo.pColorAttachmentFormats = &createInfo.colorFormat;
        createInfo.pipelineRenderingInfo.depthAttachmentFormat = vk_swapchain::depthFormat();
        createInfo.pipelineRenderingInfo.stencilAttachmentFormat = vk_swapchain::depthFormat();

        createInfo.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        createInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        createInfo.dynamicStateInfo.pDynamicStates = createInfo.dynamicStateEnables.data();
        createInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(createInfo.dynamicStateEnables.size());
        createInfo.dynamicStateInfo.flags = 0;
    }
} // namespace vk
