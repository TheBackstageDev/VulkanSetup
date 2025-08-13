#pragma once

#include <imgui/imgui.h>

#include "core/ecs.hpp"
#include "core/ecs_defines.hpp"

#include <filesystem>
#include <string>
#include <vector>

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
        asset_handler_t(std::filesystem::path rootPath, std::unique_ptr<vk::vk_device>& device);
        ~asset_handler_t();
        
        // TO DO: add image creation, and model creation;
        ASSET_RESULT createAsset(const AssetCreateInfo& createInfo);
        ASSET_RESULT deleteAsset(ecs::entity_id_t id);

        std::string getDefaultScript(std::string name);
    private:
        void initAssets();
        void findAssetsToInit(std::vector<std::filesystem::path>& images, std::vector<std::filesystem::path>& models, std::filesystem::path current);

        std::unique_ptr<vk::vk_device>& _device;

        ecs::scene_t<> _assets;
        std::filesystem::path _rootPath;

        ecs::entity_id_t selectedAsset = ecs::null_entity_id;
    };
} // namespace eng
