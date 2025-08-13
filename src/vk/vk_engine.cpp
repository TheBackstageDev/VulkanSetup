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
            _currentImage = VK_NULL_HANDLE;
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
        renderer = std::make_unique<vk_renderer>(pipeline, device, context, window, swapchain, &offscreen);

        core::input::setWindow(window->window());

        initImgui(pipelineInfo.pipelineRenderingInfo);
        createImageSet();
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
        initInfo.Queue = context.graphicsQueue;
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

        // Init UI

        fileSystem = std::make_unique<eng::file_system_t>();
        //assetHandler = std::make_unique<eng::asset_handler_t>(fileSystem->rootPath(), device);
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
        globalInfo.range = sizeof(globalUbo);

        vk_descriptordata globalData{};
        globalData.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        globalData.pBufferInfo = &globalInfo;

        globaluboChannelInfo = device->setDescriptorData(globalData);
        
        core::image_t defaultImage;
        core::imageloader_t::loadImage("C:\\Users\\gabri\\OneDrive\\Documentos\\GitHub\\VulkanSetup\\src\\resource\\textures\\default.png", &defaultImage);

        VkDescriptorImageInfo textureInfo{};
        textureInfo.sampler = defaultImage.sampler;
        textureInfo.imageView = defaultImage.view;
        textureInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        
        vk_descriptordata texData{};
        texData.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        texData.pImageInfo = &textureInfo;

        defaultTextureChannelInfo = device->setDescriptorData(texData);
    }

    void vk_engine::setupBaseScene()
    {
        renderer->setScene(_scene);
    }

    ImVec2 previousWindowSize = {0.0f, 0.0f};
    bool shouldRecreateOffscreen = false; // Temporary

    void vk_engine::runEngineUI()
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar            |
                                       ImGuiWindowFlags_NoCollapse            |
                                       ImGuiWindowFlags_NoResize              |
                                       ImGuiWindowFlags_NoMove                |
                                       ImGuiWindowFlags_NoBringToFrontOnFocus |
                                       ImGuiWindowFlags_NoNavFocus            |
                                       ImGuiWindowFlags_NoBackground          |
                                       ImGuiWindowFlags_MenuBar;

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::Begin("Engine Window", nullptr, windowFlags);

        ImGuiID dockspaceID = ImGui::GetID("EngineUi");
        ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

        ImGui::End();

        ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar);

        ImVec2 currentSize = ImGui::GetWindowSize();
        if ((previousWindowSize.x != currentSize.x) || (previousWindowSize.y != currentSize.y))
        {
            previousWindowSize = currentSize;
            shouldRecreateOffscreen = true;
        }
            
        if (_currentImage)
        {
            ImGui::Image((ImTextureID)_currentImage, currentSize);
        }
        ImGui::End();

        runObjectList();
        runInspector();
        runConsole();
        fileSystem->render();
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
            {
                _currentlySelected = _id;
                fileSystem->reset();
            }
        });

        ImGui::End();
    }

    void vk_engine::runInspector()
    {
        ImGui::Begin("Inspector");

        if (fileSystem->isFileSelected())
        {
            runFileContents();

            ImGui::End();
            return;
        }

        if (_currentlySelected == ecs::null_entity_id)
        {
            ImGui::Text("No Entity or File Selected!");
            ImGui::End();

            return;
        }

        eng::transform_t& ent_transform = _scene.get<eng::transform_t>(_currentlySelected);
        
        runTransform();

        ImGui::End();
    }

    void vk_engine::runConsole()
    {
        ImGui::Begin("Console");

        ImGui::End();
    }

    std::string formatFileTime(const std::filesystem::file_time_type& time)
    {
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            time - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now());
        auto time_t = std::chrono::system_clock::to_time_t(sctp);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M");
        return ss.str();
    }

    void vk_engine::runFileContents()
    {
        std::vector<char> fileContents = fileSystem->getFileContents();
        const eng::fileInfo file = fileSystem->getCurrent();

        ImGui::BeginChild("##File", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::Text("File: %s", file.name.c_str());
        ImGui::Separator();

        ImGui::Text("Contents");
        ImGui::InputTextMultiline("##Contents", fileContents.data(), 
                                fileContents.size() + 1, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / 3), 
                                ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_AllowTabInput);
                                
        ImGui::Text("Size: %d", file.size);
        ImGui::Text("Last Modified: %s", formatFileTime(file.lastModified).c_str());
                                
        ImGui::EndChild();
    }

    const float widthFactor = 4.0f;

    void vk_engine::runTransform()
    {
        eng::transform_t& ent_transform = _scene.get<eng::transform_t>(_currentlySelected);

        ImGui::BeginChild(ImGui::GetID("Transform"), ImVec2(0, 0), ImGuiChildFlags_AutoResizeY);

        ImGui::Text("Translation");
        ImGui::PushID("Translation");
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x / widthFactor);
        ImGui::DragFloat("X##Translation", &ent_transform.translation.x, 0.1f);
        ImGui::SameLine();
        ImGui::DragFloat("Y##Translation", &ent_transform.translation.y, 0.1f);
        ImGui::SameLine();
        ImGui::DragFloat("Z##Translation", &ent_transform.translation.z, 0.1f);
        ImGui::PopItemWidth();
        ImGui::PopID();

        ImGui::Text("Rotation");
        ImGui::PushID("Rotation");
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x / widthFactor);

        glm::vec3 eulerAngles = glm::degrees(glm::eulerAngles(ent_transform.rotation));

        bool rotationChanged = false;
        float yaw = eulerAngles.y;   
        float pitch = eulerAngles.x; 
        float roll = eulerAngles.z;  

        if (ImGui::DragFloat("Y##Rotation", &yaw, 0.1f, -180.0f, 180.0f))
            rotationChanged = true;
        ImGui::SameLine();
        if (ImGui::DragFloat("X##Rotation", &pitch, 0.1f, -90.0f, 90.0f))
            rotationChanged = true;
        ImGui::SameLine();
        if (ImGui::DragFloat("Z##Rotation", &roll, 0.1f, -180.0f, 180.0f))
            rotationChanged = true;

        if (rotationChanged)
            ent_transform.rotation = glm::quat(glm::radians(glm::vec3(pitch, yaw, roll)));

        ImGui::PopItemWidth();
        ImGui::PopID();

        ImGui::Text("Scale");
        ImGui::PushID("Scale");
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x / widthFactor);
        ImGui::DragFloat("X##Scale", &ent_transform.scale.x, 0.1f, 0.f, 100.0f);
        ImGui::SameLine();
        ImGui::DragFloat("Y##Scale", &ent_transform.scale.y, 0.1f, 0.f, 100.0f);
        ImGui::SameLine();
        ImGui::DragFloat("Z##Scale", &ent_transform.scale.z, 0.1f, 0.f, 100.0f);
        ImGui::PopItemWidth();
        ImGui::PopID();

        ImGui::EndChild();
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

    void vk_engine::createImageSet()
    {
        VkDescriptorSetLayoutBinding binding{};
        binding.binding = 0;
        binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding.descriptorCount = 1;
        binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        binding.pImmutableSamplers = nullptr;

        VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlags{};
        bindingFlags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
        bindingFlags.bindingCount = 1;
        VkDescriptorBindingFlags flags = VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
        bindingFlags.pBindingFlags = &flags;

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &binding;

        if (vkCreateDescriptorSetLayout(device->device(), &layoutInfo, nullptr, &_imageDescriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create descriptor set layout for _currentImage!");
        }

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = imguiPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &_imageDescriptorSetLayout;

        if (vkAllocateDescriptorSets(device->device(), &allocInfo, &_currentImage) != VK_SUCCESS)
        {
            vkDestroyDescriptorSetLayout(device->device(), _imageDescriptorSetLayout, nullptr);
            throw std::runtime_error("Failed to allocate descriptor set for _currentImage!");
        }
    }

    void vk_engine::createImage()
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = offscreen->getImageView();
        imageInfo.sampler = offscreen->getSampler();

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = _currentImage;
        write.dstBinding = 0;
        write.dstArrayElement = 0;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.descriptorCount = 1;
        write.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(device->device(), 1, &write, 0, nullptr);
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

            if (shouldRecreateOffscreen)
            {
                offscreen->recreate(VkExtent2D{static_cast<uint32_t>(previousWindowSize.x), static_cast<uint32_t>(previousWindowSize.y)});
                createImage();
                shouldRecreateOffscreen = false;
            }

            if (VkCommandBuffer cmd = renderer->startFrame()) 
            {
                renderer->beginOffscreenPass(cmd);
                runExecutionPipeline(cmd);
                runRendering(cmd);
                renderer->endOffscreenPass(cmd);
                
                renderer->beginRenderpass(cmd);
                runEngineUI();
                renderer->endFrame(cmd);
            }

            std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
            info.deltaTime = std::chrono::duration<float>(end - start).count();
        }
    }
} // namespace vk
