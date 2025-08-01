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

        initImgui(pipelineInfo.pipelineRenderingInfo);
        setupBuffers();
    }

    vk_application::~vk_application()
    {
        vkDeviceWaitIdle(device->device());

        ImGui_ImplGlfw_Shutdown();
        ImGui_ImplVulkan_Shutdown();
        ImGui::DestroyContext();

        vkDestroyDescriptorPool(device->device(), imguiPool, nullptr);
    }

    void vk_application::setupBuffers()
    {
        globalUbo defaultGlobalUbo{};
        defaultGlobalUbo.projection = glm::mat4(1.0f);
        defaultGlobalUbo.view = glm::mat4(1.0f);

        globalBuffer = std::make_unique<vk_buffer>(
            device,
            &defaultGlobalUbo,
            sizeof(globalUbo),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VMA_MEMORY_USAGE_CPU_TO_GPU
        );

        VkDescriptorBufferInfo globalInfo{};
        globalInfo.buffer = globalBuffer->buffer();
        globalInfo.offset = 0;
        globalInfo.range = sizeof(globalBuffer);

        vk_descriptordata globalData{};
        globalData.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        globalData.pBufferInfo = &globalInfo;

        globaluboChannelInfo = device->setDescriptorData(globalData);
        
        core::image_t defaultImage;
        core::imageloader_t::loadImage("src/resource/textures/default.png", &defaultImage, device);

        VkDescriptorImageInfo textureInfo{};
        textureInfo.sampler = defaultImage.sampler;
        textureInfo.imageView = defaultImage.view;
        textureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        
        vk_descriptordata texData{};
        texData.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        texData.pImageInfo = &textureInfo;

        std::pair<uint32_t, uint32_t> textureChannelInfo = device->setDescriptorData(texData);
    }

    void vk_application::initImgui(VkPipelineRenderingCreateInfo& pipelineRenderingInfo)
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; 
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; 
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui::StyleColorsDark(&ImGui::GetStyle());

        VkDescriptorPoolCreateInfo imguiPoolCreateInfo{};
        imguiPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        imguiPoolCreateInfo.maxSets = 1000;

        VkDescriptorPoolSize poolSizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };

        imguiPoolCreateInfo.poolSizeCount = IM_ARRAYSIZE(poolSizes);
        imguiPoolCreateInfo.pPoolSizes = poolSizes;
        imguiPoolCreateInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; 

        if (vkCreateDescriptorPool(device->device(), &imguiPoolCreateInfo, nullptr, &imguiPool) != VK_SUCCESS)
        {
            throw std::runtime_error("Unable to create imgui descriptor pool!");
        }
                
        ImGui_ImplVulkan_InitInfo initInfo{};
        initInfo.Instance = context.vk_instance();
        initInfo.PhysicalDevice = device->phydevice();
        initInfo.Device = device->device();
        initInfo.QueueFamily = device->graphicsFamily();
        initInfo.Queue = device->graphicsQueue();
        initInfo.PipelineRenderingCreateInfo = pipelineRenderingInfo;
        initInfo.PipelineCache = VK_NULL_HANDLE;
        initInfo.DescriptorPool = imguiPool;
        initInfo.MinImageCount = swapchain->imageAmmount();
        initInfo.ImageCount = swapchain->imageAmmount();
        initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        initInfo.UseDynamicRendering = true;

        ImGui_ImplVulkan_LoadFunctions(
            VK_API_VERSION_1_3,
            [](const char* function_name, void* user_data) -> PFN_vkVoidFunction 
            {
                return glfwGetInstanceProcAddress(static_cast<VkInstance>(user_data), function_name);
            }
        );

        ImGui_ImplGlfw_InitForVulkan(window->window(), true);
        ImGui_ImplVulkan_Init(&initInfo);
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
    
        eng::camera_t cam{scene};
        renderer->setScene(scene);

        eng::model_t model;
        eng::modelloader_t::loadModel("src/resource/suzane.obj", &model, device);

        ecs::entity_id_t modelId = scene.create();
        scene.construct<eng::transform_t>(modelId).translation = {0.0f, 0.0f, 0.0f};
        scene.construct<eng::model_t>(modelId) = model;
        scene.construct<eng::texture_t>(modelId) = {defaultTextureChannelInfo.second, defaultTextureChannelInfo.first};

        globalUbo globalubo{};

        while (!window->should_close())
        {
            glfwPollEvents();

            if (VkCommandBuffer cmd = renderer->startFrame()) 
            {
                auto& info = renderer->getFrameInfo();
                info.channelIndices = device->getChannelInfo();

                ImGui::ShowDemoWindow(nullptr);

                std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

                cam.perspective(70.f, renderer->aspectRatio());

                eng::transform_t& camTransform = scene.get<eng::transform_t>(cam.getId());
                handleCamInput(camTransform);
    
                globalubo.projection = cam.getProjection();
                globalubo.view = cam.getView();
                
                globalBuffer->update(&globalubo);
                globalBuffer->bindUniform(cmd, pipeline->layout(), device, globaluboChannelInfo);

                renderer->renderScene();
                renderer->endFrame(cmd);

                std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

                info.deltaTime = std::chrono::duration<double>(end - start).count();
            }
        }

        vkDeviceWaitIdle(device->device());
    }
} // namespace vk
