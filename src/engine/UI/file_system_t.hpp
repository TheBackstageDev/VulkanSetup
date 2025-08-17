#pragma once

#include <volk/volk.h>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_vulkan.h>

#include <filesystem>

#include <unordered_map>
#include <map>

#include "core/imageloader.hpp"
#include "core/input.hpp"

namespace eng
{
    struct fileInfo
    {
        std::string name;                  
        std::filesystem::path path;        
        uint64_t size;                      // File size in bytes (for display)
        std::string extension;              // File extension (e.g., "txt", "png") for icon mapping
        std::filesystem::file_time_type lastModified; 
        VkDescriptorSet icon;                                          
    };
    
    struct dirInfo
    {
        std::string name;
        std::filesystem::path dirPath;
        std::filesystem::path parentPath;
        uint32_t depth;

        bool open = false;
        std::vector<fileInfo> files;

        bool isParent(std::filesystem::path path) { return path == parentPath; }

        bool operator<(const dirInfo& other) const {
            return std::tie(depth, name) < std::tie(other.depth, other.name);
        }
    };

    class file_system_t
    {
    public:
        file_system_t(std::filesystem::path rootPath = std::filesystem::current_path() / "assets");
        ~file_system_t();
        
        void render();

        bool isFileSelected() const { return fileSelected; }
        void update() { updatePath(_rootPath); }
        void reset() { _currentPath = ""; fileSelected = false; }
        
        std::vector<char> getFileContents(); 
        
        fileInfo getCurrent() const
        {
            if (!fileSelected) {
                return fileInfo{}; 
            }

            for (const auto& [path, dir] : _systemInfo) {
                auto it = std::find_if(dir.files.begin(), dir.files.end(),
                    [this](const auto& file) {
                        return file.path == _currentPath;
                    });
                if (it != dir.files.end()) {
                    return *it; 
                }
            }

            return fileInfo{}; 
        }

        std::filesystem::path rootPath() const { return _rootPath; }

        bool isTextFileFormats(std::string extension) { return std::find(_textExtensions.begin(), _textExtensions.end(), extension) != _textExtensions.end(); }
        bool isImageFileFormats(std::string extension) { return std::find(_imageExtensions.begin(), _imageExtensions.end(), extension) != _imageExtensions.end(); }

    private:
        void getSystemInfo();
        dirInfo getDirInfo(std::map<std::string, dirInfo>& systemInfo, std::filesystem::path current = "", size_t depth = 0, std::filesystem::path parent = "");
        fileInfo getFileInfo(std::filesystem::path filePath);

        void updatePath(std::filesystem::path path);
        void initIcons();
        void initSet(core::image_t& icon, std::string name);

        void filePopup(const fileInfo& file);
        void dirPopup(dirInfo& dir);

        void dirRename(dirInfo& dir);
        void fileRename(fileInfo& dir);

        const std::filesystem::path _rootPath;
        std::filesystem::path _currentPath = "";
        std::filesystem::path _newPath = "";
        
        std::map<std::string, dirInfo> _systemInfo;
        std::unordered_map<std::string, VkDescriptorSet> _icons;
        
        bool renaming = false;
        bool fileSelected = false;
        bool updatePathDebounce = false;

        const std::vector<std::string> _imageExtensions = 
            {".png", ".jpg", ".jpeg"};

        const std::vector<std::string> _textExtensions = 
            {".txt", ".hpp", ".cpp"};
    }; 
} // namespace eng

