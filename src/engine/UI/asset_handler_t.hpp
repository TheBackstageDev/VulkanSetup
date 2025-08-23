#pragma once

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <efsw/include/efsw/efsw.hpp>

#include "core/ecs.hpp"
#include "core/ecs_defines.hpp"
#include "core/imageloader.hpp"
#include "engine/modelloader_t.hpp"

#include "vk/vk_device.hpp"
#include "file_system_t.hpp"

#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>

#include <thread>
#include <mutex>
#include <queue>

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
        std::filesystem::path path;
    };

    class asset_handler_t : public efsw::FileWatchListener
    {
    public:
        struct asset_t
        {
            std::string name;
            vk::vk_channelindices indices;

            uint32_t id;
        };

        asset_handler_t(std::filesystem::path rootPath, std::unique_ptr<vk::vk_device>& device);
        ~asset_handler_t();

        ASSET_RESULT createAsset(const AssetCreateInfo& createInfo);
        ASSET_RESULT deleteAsset(const std::filesystem::path path);

        std::string getDefaultScript(std::string name);
        
        vk::vk_channelindices getIndices(std::filesystem::path path) const 
        { 
            if (_assets.find(path) == _assets.end())
            {
                return vk::vk_channelindices();    
            }

            return _assets.at(path).indices;
        }

        eng::model_t getModel(std::filesystem::path path) 
        { 
            if (_assets.find(path) == _assets.end())
            {
                return eng::model_t();    
            }

            return _models.at(path);
        }

        VkDescriptorSet getTexture(std::filesystem::path path)
        {
            if (_assets.find(path) == _assets.end())
            {
                return VK_NULL_HANDLE;    
            }
            
            return _images.at(path);
        }

        std::unordered_map<std::filesystem::path, VkDescriptorSet>& getTextures() { return _images; }
        std::unordered_map<std::filesystem::path, eng::model_t>& getModels() { return _models; }

        void handleFileAction(efsw::WatchID watchid, const std::string& dir, const std::string& filename,
            efsw::Action action, std::string old_filename) override;
        
        bool handleEvents();
    private:
        void initAssets();
        void findAssetsToInit(std::vector<std::filesystem::path>& images, std::vector<std::filesystem::path>& models, std::filesystem::path current);
        
        struct FileEvent 
        {
            efsw::Action action;
            std::string dir;
            std::string filename;
            std::string old_filename;
        };

        std::mutex _eventMutex;
        std::queue<FileEvent> _eventQueue;

        std::unordered_map<std::filesystem::path, asset_t> _assets;
        std::unordered_map<std::filesystem::path, VkDescriptorSet> _images;
        std::unordered_map<std::filesystem::path, eng::model_t> _models;

        std::unique_ptr<vk::vk_device>& _device;
        efsw::FileWatcher _watcher;

        const std::filesystem::path _rootPath;
    };
} // namespace eng
