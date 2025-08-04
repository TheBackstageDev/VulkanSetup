#include "vk_engine.hpp"
#include "core/actor_registry.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <chrono>

namespace vk
{
    vk_engine::vk_engine()
    {
        initVulkan();
        setupBuffers();

        setupBaseScene();
    }

    vk_engine::~vk_engine()
    {
        cleanup();
    }

    void vk_engine::cleanup()
    {
        vkDeviceWaitIdle(device->device());

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        if (imguiPool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(device->device(), imguiPool, nullptr);
            imguiPool = VK_NULL_HANDLE;
        }
    }
    
    void vk_engine::initVulkan()
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

        const std::string pathToVertex = "C:\\Users\\gabri\\OneDrive\\Documentos\\GitHub\\VulkanSetup\\src\\shaders\\test.vert.spv";
        const std::string pathToFragment = "C:\\Users\\gabri\\OneDrive\\Documentos\\GitHub\\VulkanSetup\\src\\shaders\\test.frag.spv";

        pipelineCreateInfo pipelineInfo{};
        vk_pipeline::defaultPipelineCreateInfo(pipelineInfo);
        pipelineInfo.descriptorSetLayouts = device->getSetLayouts();

        pipeline = std::make_unique<vk_pipeline>(device, swapchain, pathToVertex, pathToFragment, pipelineInfo);
        offscreen = std::make_unique<vk_offscreen_renderer>(swapchain->imageAmmount(), swapchain->extent());
        renderer = std::make_unique<vk_renderer>(pipeline, device, context, window, swapchain, nullptr);

        core::input::setWindow(window->window());

        initImgui(pipelineInfo.pipelineRenderingInfo);
        setupBuffers();
    }

    void vk_engine::initImgui(const VkPipelineRenderingCreateInfo& pipelineRenderingInfo)
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

    void vk_engine::setupBuffers()
    {
        globalUbo defaultGlobalUbo{};
        defaultGlobalUbo.projection = glm::mat4(1.0f);
        defaultGlobalUbo.view = glm::mat4(1.0f);

        globalBuffer = std::make_unique<vk_buffer>(
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
        core::imageloader_t::loadImage("C:\\Users\\gabri\\OneDrive\\Documentos\\GitHub\\VulkanSetup\\src\\resource\\textures\\default.png", &defaultImage, device);

        VkDescriptorImageInfo textureInfo{};
        textureInfo.sampler = defaultImage.sampler;
        textureInfo.imageView = defaultImage.view;
        textureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        
        vk_descriptordata texData{};
        texData.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        texData.pImageInfo = &textureInfo;

        std::pair<uint32_t, uint32_t> textureChannelInfo = device->setDescriptorData(texData);
    }

    void vk_engine::setupBaseScene()
    {
        renderer->setScene(_scene);
    }

    ImVec2 previousWindowSize = {0.0f, 0.0f};

    void vk_engine::runEngineUI()
    {
        ImGui::Begin("Engine Window");

        ImVec2 currentSize = ImGui::GetWindowSize();
        if ((previousWindowSize.x != currentSize.x) && (previousWindowSize.y != currentSize.y))
        {
            offscreen->recreate(VkExtent2D{static_cast<uint32_t>(currentSize.x), static_cast<uint32_t>(currentSize.y)});
            previousWindowSize = currentSize;
        }
        
        ImGuiID dockspace_id = ImGui::GetID("EngineUI");
        ImGui::DockSpace(dockspace_id);

        ImGui::End();

        runObjectList();
        runProperties();
        runConsole();
    }

    void vk_engine::runObjectList()
    {
        ImGui::Begin("Scene");

        _scene.for_all<eng::transform_t>([&](ecs::entity_id_t& _id, eng::transform_t& _transform) 
        {
            std::string label = "Entity Doe";
            if (_scene.has<core::name_t>(_id))
                label = _scene.get<core::name_t>(_id).name;

            label = std::string(label + "##" + std::to_string(_id));

            if (ImGui::Selectable(label.c_str(), _currentlySelected == _id)) 
                _currentlySelected = _id;
        });

        ImGui::End();
    }

    void vk_engine::runProperties()
    {
        ImGui::Begin("Properties");

        if (_currentlySelected == ecs::null_entity_id)
        {
            ImGui::Text("No Entity Selected!");
            ImGui::End();

            return;
        }

        eng::transform_t& ent_transform = _scene.get<eng::transform_t>(_currentlySelected);
        
        ImGui::DragFloat3("Translation", glm::value_ptr(ent_transform.translation), 0.1f);
        ImGui::DragFloat3("Rotation", glm::value_ptr(ent_transform.rotation), 0.1f);
        ImGui::DragFloat3("Scale", glm::value_ptr(ent_transform.scale), 0.1f);

        ImGui::End();
    }

    void vk_engine::runConsole()
    {
        ImGui::Begin("Console");

        ImGui::End();
    }

    void vk_engine::runRendering(VkCommandBuffer cmd)
    {
        globalUbo globalubo{};
        cam.perspective(80.f, renderer->aspectRatio());
        //cam.ortho(-renderer->aspectRatio(), renderer->aspectRatio(), -1.0f, 1.0f, 0.1f, 10.f);

        globalubo.projection = cam.getProjection();
        globalubo.view = cam.getView();
                    
        globalBuffer->update(&globalubo);
        globalBuffer->bindUniform(cmd, pipeline->layout(), device, globaluboChannelInfo);

        renderer->renderScene();
    }

    void vk_engine::runExecutionPipeline(VkCommandBuffer cmd)
    {
        auto& info = renderer->getFrameInfo();

        for (auto& actor : _actors)
            actor->Update(info.deltaTime);
            
        _actors.erase(std::remove_if(_actors.begin(), _actors.end(),
            [](const std::unique_ptr<core::systemactor>& actor) {
                return actor->ShouldDestroy();
            }), _actors.end());
            
        for (auto& actor : _actors)
            actor->OnRender(cmd);

        for (auto& actor : _actors)
            actor->LateUpdate(info.deltaTime);
    }

    void vk_engine::runMainLoop()
    {
        for (auto& ctor : core::getActorRegistry()) 
            ctor(*this, _scene); 
        
        for (auto& actor : _actors)
            actor->Awake();

        for (auto& actor : _actors)
            actor->Start();

        while (!window->should_close())
        {
            glfwPollEvents();
            auto& info = renderer->getFrameInfo();
            info.channelIndices = device->getChannelInfo();

            std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

            if (VkCommandBuffer cmd = renderer->startFrame()) 
            {
                runExecutionPipeline(cmd);
                runRendering(cmd);
                runEngineUI();

                renderer->endFrame(cmd);
            }

            std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
            info.deltaTime = std::chrono::duration<float>(end - start).count();
        }
    }
} // namespace vk
