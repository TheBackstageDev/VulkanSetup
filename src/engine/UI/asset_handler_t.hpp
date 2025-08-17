#pragma once

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_vulkan.h>

#include "core/ecs.hpp"
#include "core/ecs_defines.hpp"

#include "core/imageloader.hpp"
#include "engine/modelloader_t.hpp"

#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>

#include "vk/vk_device.hpp"

namespace eng
{
    enum class ASSET_RESULT
    {
        ASSET_RESULT_NONE = 0,
        ASSET_RESULT_SUCCESS,
        ASSET_RESULT_FAILURE
    };

    struct AssetCreateInfo
    {
        std::string name;
        std::string extension;
        std::filesystem::path path;

        const char* pData;
    };

    class asset_handler_t
    {
    public:
        struct asset_t
        {
            std::string name;
            std::filesystem::path path;
            vk::vk_channelindices indices;

            uint32_t id;
        };

        asset_handler_t(std::filesystem::path rootPath, std::unique_ptr<vk::vk_device>& device);
        ~asset_handler_t();
        
        // TO DO: add image creation, and model creation;
        ASSET_RESULT createAsset(const AssetCreateInfo& createInfo);
        ASSET_RESULT deleteAsset(uint32_t id);

        std::string getDefaultScript(std::string name);
        
        vk::vk_channelindices getIndices(std::filesystem::path path) const 
        { 
            return std::find_if(_assets.begin(), _assets.end(), [&](const asset_t& asset) 
            {
                return asset.path == path;
            })->indices;  
        }

        eng::model_t getModel(std::filesystem::path path) 
        { 
            return _models.at(path);
        }

        VkDescriptorSet getTexture(std::filesystem::path path)
        {
            return _images.at(path);
        }
    private:
        void initAssets();
        void findAssetsToInit(std::vector<std::filesystem::path>& images, std::vector<std::filesystem::path>& models, std::filesystem::path current);

        std::vector<asset_t> _assets;

        std::unordered_map<std::filesystem::path, VkDescriptorSet> _images;
        std::unordered_map<std::filesystem::path, eng::model_t> _models;

        std::unique_ptr<vk::vk_device>& _device;
        std::filesystem::path _rootPath;
    };
} // namespace eng
