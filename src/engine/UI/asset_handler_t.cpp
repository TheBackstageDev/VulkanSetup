#include "asset_handler_t.hpp"

#include <sstream>
#include <fstream>

namespace eng
{
    static uint32_t lastId = 0;
    asset_handler_t::asset_handler_t(std::filesystem::path rootPath, std::unique_ptr<vk::vk_device>& device)
        : _rootPath(rootPath), _device(device)
    {
        initAssets();

        _watcher.addWatch(rootPath.string(), this, true); 
        _watcher.watch();
    }

    asset_handler_t::~asset_handler_t()
    {
    }

    void asset_handler_t::handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename,
                        efsw::Action action, std::string old_filename)
    {
        std::lock_guard<std::mutex> lock(_eventMutex);
        _eventQueue.push(FileEvent{action, dir, filename, old_filename});
    }

    bool asset_handler_t::handleEvents()
    {
        bool hasChanged = false;

        while (!_eventQueue.empty())
        {
            auto& event = _eventQueue.front();
            
            const std::filesystem::path new_path = std::filesystem::path(event.dir) / event.filename;
            std::filesystem::path old_path = std::filesystem::path(event.dir) / event.old_filename;

            hasChanged = true;

            switch ( event.action ) {
                case efsw::Actions::Add:
                    if (std::filesystem::exists(new_path))
                    {
                        asset_t newAsset{
                            .name = std::string(event.filename),
                            .id = ++lastId
                        };

                        const std::string extension = new_path.extension().string();

                        if (file_system_t::isImageFileFormats(extension))
                        {
                            core::image_t image;
                            if (!core::imageloader_t::loadImage(new_path.string(), &image))
                            {
                                --lastId;
                                return false;
                            }

                            VkDescriptorImageInfo imageInfo{};
                            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                            imageInfo.imageView = image.view;
                            imageInfo.sampler = image.sampler; 

                            vk::vk_descriptordata imageData{};
                            imageData.pImageInfo = &imageInfo;
                            imageData.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

                            _images.emplace(new_path, ImGui_ImplVulkan_AddTexture(image.sampler, image.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));

                            newAsset.indices = _device->setDescriptorData(imageData);  
                        }
                        else if (file_system_t::isModelFileFormats(extension))
                        {
                            eng::model_t model;
                            eng::modelloader_t::loadModel(new_path.string(), &model);

                            asset_t asset{};
                            asset.name = event.filename;
                            asset.id = ++lastId;
                            
                            _models.emplace(new_path, std::move(model));
                        }

                        _assets.emplace(new_path, std::move(newAsset));
                    }
                    break;
                case efsw::Actions::Delete:
                    if (_assets.find(new_path) != _assets.end())
                    {
                        if (_models.find(new_path) != _models.end())
                        {
                            _models.erase(new_path);
                        }
                        else if (_images.find(new_path) != _images.end())
                        {
                            vk::vk_channelindices& indices = _assets.at(new_path).indices;
                            
                            _device->freeDescriptorData(indices.index, indices.channelIndex);
                            ImGui_ImplVulkan_RemoveTexture(_images.at(new_path));
                            _images.erase(new_path);
                        }

                        _assets.erase(new_path);
                    }
                    break;
                case efsw::Actions::Modified:
                    break;
                case efsw::Actions::Moved:
                    if (auto it = _assets.find(old_path); it != _assets.end()) {
                        asset_t asset = it->second;
                        asset.name = event.filename;
                        _assets.erase(it);
                        _assets[new_path] = asset;

                        if (auto img_it = _images.find(old_path); img_it != _images.end()) {
                            VkDescriptorSet texture = img_it->second;
                            _images.erase(img_it);
                            _images[new_path] = texture;
                        }

                        if (auto model_it = _models.find(old_path); model_it != _models.end()) {
                            eng::model_t model = model_it->second;
                            _models.erase(model_it);
                            _models[new_path] = model;
                        }
                    }
                    break;
                default:
                    std::cout << "Should never happen!" << std::endl;
            }
            _eventQueue.pop();
        }

        return hasChanged;
    }

    void asset_handler_t::initAssets()
    {
        std::vector<std::filesystem::path> imagesToPrepare;
        std::vector<std::filesystem::path> modelsToPrepare;

        findAssetsToInit(imagesToPrepare, modelsToPrepare, _rootPath);

        for (auto& path : imagesToPrepare)
        {
            core::image_t image;
            core::imageloader_t::loadImage(path.string(), &image);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = image.view;
            imageInfo.sampler = image.sampler; 

            vk::vk_descriptordata imageData{};
            imageData.pImageInfo = &imageInfo;
            imageData.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

            _images.emplace(path, ImGui_ImplVulkan_AddTexture(image.sampler, image.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));

            asset_t asset{};
            asset.name = path.filename().string();
            asset.id = ++lastId;
            asset.indices = _device->setDescriptorData(imageData);

            _assets.emplace(path, std::move(asset));
        }

        for (auto& path : modelsToPrepare)
        {
            eng::model_t model;
            eng::modelloader_t::loadModel(path.string(), &model);

            asset_t asset{};
            asset.name = path.filename().string();
            asset.id = ++lastId;
            
            _assets.emplace(path, std::move(asset));
            _models.emplace(path, std::move(model));
        }
    }

    void asset_handler_t::findAssetsToInit(std::vector<std::filesystem::path>& images, std::vector<std::filesystem::path>& models, std::filesystem::path current)
    {
        for (const auto& entry : std::filesystem::directory_iterator(current))
        {
            if (entry.is_directory())
            {
                findAssetsToInit(images, models, entry.path());
                continue;
            }
            
            const std::string extension = entry.path().extension().string();

            if (file_system_t::isImageFileFormats(extension))
            {
                images.push_back(entry.path());
                continue;
            }
            
            if (file_system_t::isModelFileFormats(extension))
            {
                models.push_back(entry.path());
                continue;
            }
        }
    }

    ASSET_RESULT asset_handler_t::createAsset(const AssetCreateInfo& create_info)
    {
        try
        {
            const std::string name = create_info.path.filename().string();
            const std::string extension = create_info.path.extension().string();

            const std::filesystem::path path = create_info.path;

            if (name.empty() || name.find_first_of("\\/:*?\"<>|") != std::string::npos) {
                std::cerr << "Error: Invalid asset name '" << name << "'" << std::endl;
                return ASSET_RESULT::ASSET_RESULT_FAILURE;
            }

            asset_t asset{};
            asset.name = name;
            asset.id = ++lastId;

            if (file_system_t::isImageFileFormats(extension))
            {
                core::image_t image;
                core::imageloader_t::loadImage(path.string(), &image);

                VkDescriptorImageInfo imageInfo{};
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView = image.view;
                imageInfo.sampler = image.sampler; 

                vk::vk_descriptordata imageData{};
                imageData.pImageInfo = &imageInfo;
                imageData.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

                asset.indices = _device->setDescriptorData(imageData);
                _images.emplace(path, ImGui_ImplVulkan_AddTexture(image.sampler, image.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
            }
            else if (file_system_t::isTextFileFormats(extension))
            {
                std::ofstream newFile(path);

                if (!newFile.is_open())
                {
                    std::cerr << "Failed to open file at asset creation! \n";
                    return ASSET_RESULT::ASSET_RESULT_FAILURE;
                }

                if (extension == ".txt")
                {
                    newFile.close();
                }
                else
                {
                    newFile << getDefaultScript(name);
                    newFile.close();
                }
            }
            else if (file_system_t::isModelFileFormats(extension))
            {
                eng::model_t newModel;
                if (!eng::modelloader_t::loadModel(path.string(), &newModel))
                    return ASSET_RESULT::ASSET_RESULT_FAILURE;

                _models.emplace(path, std::move(newModel));
            }

            _assets.emplace(path, std::move(asset));
            return ASSET_RESULT::ASSET_RESULT_SUCCESS;
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            return ASSET_RESULT::ASSET_RESULT_FAILURE;
        }
    }

    ASSET_RESULT asset_handler_t::deleteAsset(const std::filesystem::path path)
    {
        try
        {
            if (!std::filesystem::remove(path))
                return ASSET_RESULT::ASSET_RESULT_FAILURE;

            const std::string extension = path.extension().string();
            if (file_system_t::isImageFileFormats(extension))
            {
                vk::vk_channelindices& indices = _assets.at(path).indices;
                _device->freeDescriptorData(indices.index, indices.channelIndex);

                ImGui_ImplVulkan_RemoveTexture(_images.at(path));
                _images.erase(path);
            }
            else if (file_system_t::isModelFileFormats(extension))
            {
                _models.erase(path);
            }

            _assets.erase(path);
            return ASSET_RESULT::ASSET_RESULT_SUCCESS;
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            return ASSET_RESULT::ASSET_RESULT_FAILURE;
        }
    }

    std::string asset_handler_t::getDefaultScript(std::string name)
    {
        std::ostringstream ss;
        ss << "#include <core/systemactor.hpp>\n"
        << "#include <core/actor_registry.hpp>\n\n"
        << "using namespace vk;\n"
        << "using core::systemactor;\n\n"
        << "#include <iostream>\n\n"
        << "class " << name << " : public systemactor\n"
        << "{\n"
        << "public:\n\n"
        << "    // runs before the first frame\n"
        << "    void Awake() override\n"
        << "    {\n"
        << "        std::cout << \"Just woke up!\" << std::endl;\n"
        << "    }\n\n"
        << "    // runs every frame\n"
        << "    void Update(float dt) override\n"
        << "    {\n"
        << "        std::cout << \"Updated me!\" << std::endl;\n"
        << "    }\n\n"
        << "private:\n"
        << "    // Private data such as entity ID's...\n"
        << "};\n\n"
        << "REGISTER_ACTOR(" << name << ");\n";

        return ss.str();
    }
} // namespace eng
