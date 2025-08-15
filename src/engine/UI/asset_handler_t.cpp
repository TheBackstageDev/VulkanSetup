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
    }

    asset_handler_t::~asset_handler_t()
    {
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
            asset.path = path;
            asset.id = ++lastId;
            asset.indices = _device->setDescriptorData(imageData);

            _assets.emplace_back(std::move(asset));
        }

        for (auto& path : modelsToPrepare)
        {
            eng::model_t model;
            eng::modelloader_t::loadModel(path.string(), &model);

            asset_t asset{};
            asset.name = path.filename().string();
            asset.path = path;
            asset.id = ++lastId;
            
            _assets.emplace_back(std::move(asset));
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
            
            std::filesystem::path extension = entry.path().extension();

            if (extension == ".png" || extension == ".jpeg" || extension == ".jpg" || extension == ".hdr")
            {
                images.push_back(entry.path());
                continue;
            }
            
            if (extension == ".obj")
            {
                models.push_back(entry.path());
                continue;
            }
        }
    }

    // TO DO: add image creation, and model creation;
    ASSET_RESULT asset_handler_t::createAsset(const AssetCreateInfo& create_info)
    {
        try
        {
            if (create_info.name.empty() || create_info.name.find_first_of("\\/:*?\"<>|") != std::string::npos) {
                std::cerr << "Error: Invalid asset name '" << create_info.name << "'" << std::endl;
                return ASSET_RESULT::ASSET_RESULT_FAILURE;
            }

            std::ofstream newFile(create_info.name + create_info.extension, std::ios::binary);

            newFile << create_info.pData;

            newFile.close();


            return ASSET_RESULT::ASSET_RESULT_SUCCESS;
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            return ASSET_RESULT::ASSET_RESULT_FAILURE;
        }
    }

    ASSET_RESULT asset_handler_t::deleteAsset(uint32_t id)
    {
        try
        {
            if (!std::filesystem::remove(_assets.at(id).path))
                return ASSET_RESULT::ASSET_RESULT_FAILURE;

            _assets.erase(std::find_if(_assets.begin(), _assets.end(), [&](const asset_t& asset) { return asset.id == id; }) );

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
