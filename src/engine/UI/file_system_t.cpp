#include "file_system_t.hpp"

#include <iostream>
#include <stdexcept>

#include <fstream>

namespace eng
{
    #define FILE "file"
    #define SCRIPT "script"
    #define FOLDER "folder"
    #define IMAGE "image"
    #define MODEL "model"

    #define ICON_SIZE 16

    const std::vector<std::string> file_system_t::_textExtensions = 
            {".txt", ".hpp", ".cpp"};

    const std::vector<std::string> file_system_t::_imageExtensions = 
            {".png", ".jpg", ".jpeg"};

    const std::vector<std::string> file_system_t::_modelExtensions = 
        {".obj"};

    file_system_t::file_system_t(std::filesystem::path rootPath)
        : _rootPath(rootPath),
          _currentPath(_rootPath)
    {
        std::string root = _rootPath.string();

        if (!std::filesystem::exists(rootPath))
            std::filesystem::create_directory(rootPath);

        initIcons();
        getSystemInfo();
    }

    file_system_t::~file_system_t()
    {
    }

    void file_system_t::initIcons()
    {
        std::filesystem::path iconsFolder = "icons";

        std::filesystem::path folderIconPath = iconsFolder / "folder.png";
        std::filesystem::path fileIconPath   = iconsFolder / "file.png";
        std::filesystem::path scriptIconPath = iconsFolder / "script.png";
        std::filesystem::path imageIconPath = iconsFolder / "image.png";
        std::filesystem::path modelIconPath = iconsFolder / "model.png";

        core::image_t folderIcon;
        core::image_t fileIcon;
        core::image_t scriptIcon;
        core::image_t imageIcon;
        core::image_t modelIcon;

        core::imageloader_t::loadImage(folderIconPath.string(), &folderIcon);
        core::imageloader_t::loadImage(fileIconPath.string(), &fileIcon);
        core::imageloader_t::loadImage(scriptIconPath.string(), &scriptIcon);
        core::imageloader_t::loadImage(imageIconPath.string(), &imageIcon);
        core::imageloader_t::loadImage(modelIconPath.string(), &modelIcon);

        initSet(folderIcon, "folder");
        initSet(fileIcon, "file");
        initSet(scriptIcon, "script");
        initSet(imageIcon, "image");
        initSet(modelIcon, "model");
    }

    void file_system_t::initSet(core::image_t& icon, std::string name)
    {
        _icons.emplace(name, ImGui_ImplVulkan_AddTexture(icon.sampler, icon.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
    }

    std::vector<char> file_system_t::getFileContents()
    {
        if (!isTextFileFormats(_currentPath.extension().string()))
            return {'\0'};

        std::ifstream file{_currentPath, std::ios::ate | std::ios::binary};

        if (!file.is_open())
        {
            return {'f', 'a', 'i', 'l', 'e', 'd', ' ', 't', 'o', ' ', 'o', 'p', 'e', 'n', ' ', 'f', 'i', 'l', 'e', '!', '\0'};
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        if (fileSize == 0)
            return {'e', 'm', 'p', 't', 'y', '.', '\0'};

        buffer.push_back('\0');
        return buffer;
    }

    void file_system_t::dirPopup(dirInfo& dir)
    {
        if (ImGui::BeginPopup("##DirEdit"))
        {
            if (ImGui::MenuItem("Create File"))
            {
                std::filesystem::path newPath = dir.dirPath / "NoName.txt";

                std::ofstream newFile(newPath);
                newFile.close();

                _newPath = newPath;
                _currentPath = dir.dirPath;
                updatePathDebounce = true;
            }

            if (ImGui::MenuItem("Create Directory"))
            {
                std::filesystem::path newPath = dir.dirPath / "NoNameDir";
                std::filesystem::create_directory(newPath);

                _newPath = newPath;
                _currentPath = dir.dirPath;
                updatePathDebounce = true;
            }

            if (ImGui::MenuItem("Rename"))
            {
                renaming = true;
                _currentPath = dir.dirPath;
            }
            
            if (ImGui::MenuItem("Delete"))
            {
                std::filesystem::remove_all(_currentPath);

                _currentPath = _rootPath;
                updatePathDebounce = true;
            }
            
            ImGui::EndPopup();
        }
    }

    void file_system_t::filePopup(const fileInfo& file)
    {
        if (ImGui::BeginPopup("##FileEdit"))
        {
            if (ImGui::MenuItem("Rename"))
            {
                renaming = true;
                _currentPath = file.path;
            }
            
            if (ImGui::MenuItem("Delete"))
            {
                std::filesystem::remove(_currentPath);

                _currentPath = _rootPath;
                updatePathDebounce = true;
            }
            
            ImGui::EndPopup();
        }
    }

    static char newName[256];
    void file_system_t::dirRename(dirInfo& dir)
    {
        strcpy_s(newName, dir.name.c_str());

        ImGui::SetKeyboardFocusHere();
        if (ImGui::InputText("##RenameInput", newName, sizeof(newName), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            std::filesystem::path newPath = dir.parentPath / newName;

            try {
                std::filesystem::path pathToRename = _newPath.empty() ? _currentPath : _newPath;

                std::filesystem::rename(pathToRename, newPath);
                pathToRename = newPath;
                updatePathDebounce = true;
            } catch (const std::exception& e) {
                std::cerr << "Rename failed: " << e.what() << std::endl;
            }

            renaming = false;
            updatePathDebounce = true;

            _currentPath = _rootPath;
            _newPath.clear();
        }
    }

    void file_system_t::fileRename(fileInfo& file)
    {
        strcpy_s(newName, file.name.c_str());

        ImGui::SetKeyboardFocusHere();
        if (ImGui::InputText("##RenameInput", newName, sizeof(newName), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            std::filesystem::path newPath = _newPath.empty() ? _currentPath.parent_path() / newName : _currentPath / newName;

            try {
                std::filesystem::path pathToRename = _newPath.empty() ? _currentPath : _newPath;

                std::filesystem::rename(pathToRename, newPath);
                pathToRename = newPath;
                updatePathDebounce = true;
            } catch (const std::exception& e) {
                std::cerr << "Rename failed: " << e.what() << std::endl;
            }

            renaming = false;
            updatePathDebounce = true;

            _currentPath = _rootPath;
            _newPath.clear();
        }
    }

    void file_system_t::render()
    {
        ImGui::Begin("File System", nullptr);

        ImGui::PushID(ImGui::GetID(_rootPath.string().c_str()));
        ImGuiTreeNodeFlags rootFlags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_DefaultOpen;

        if (_currentPath == _rootPath) 
        {
            rootFlags |= ImGuiTreeNodeFlags_Selected;
        }
        bool rootNodeOpen = ImGui::TreeNodeEx("", rootFlags);

        if (ImGui::IsItemClicked())
        {
            updatePathDebounce = true;
            _currentPath = _rootPath;
        }

        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        {    
            _currentPath = _rootPath;
            ImGui::OpenPopup("##DirEdit");
        }

        dirPopup(_systemInfo.at(_rootPath.string()));

        _systemInfo.at(_rootPath.string()).open = rootNodeOpen;

        ImGui::SameLine(0.0f, 0.0f);
        ImGui::Image((ImTextureID)_icons.at(FOLDER), ImVec2(ICON_SIZE, ICON_SIZE));
        ImGui::SameLine();
        ImGui::Text("assets");

        if (updatePathDebounce)
        {
            updatePath(_currentPath);
            updatePathDebounce = false;
        }

        if (rootNodeOpen)
        {
            for (auto& [path, dir] : _systemInfo)
            {
                if (dir.name == "assets")
                    continue;

                if (!_systemInfo.at(dir.parentPath.string()).open && dir.parentPath != _rootPath)
                    continue;

                float indent = dir.parentPath.empty() ? dir.depth * 5.0f : (dir.depth + 1) * 5.0f;

                ImGui::PushID(ImGui::GetID(dir.name.c_str()));
                ImGui::Indent(indent);
                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen;
                
                if (dir.files.empty()) 
                {
                    flags |= ImGuiTreeNodeFlags_Leaf;
                }
                if (_currentPath.string() == path) 
                {
                    flags |= ImGuiTreeNodeFlags_Selected;
                }

                bool isDirOpen = ImGui::TreeNodeEx(("##" + path).c_str(), flags | ImGuiTreeNodeFlags_SpanFullWidth, "");
                dir.open = isDirOpen;

                if (ImGui::IsItemClicked())
                {
                    _currentPath = path;
                    updatePathDebounce = true;
                }

                if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                { 
                    _currentPath = path;
                    ImGui::OpenPopup("##DirEdit");
                }

                ImGui::SameLine(0.0f, 0.0f);
                ImGui::Image((ImTextureID)_icons.at(FOLDER), ImVec2(ICON_SIZE, ICON_SIZE));
                ImGui::SameLine();

                if ((renaming && _currentPath == path) || path == _newPath)
                {
                    dirRename(dir);
                }
                else
                {
                    ImGui::Text("%s", dir.name.c_str());
                }

                dirPopup(dir);
                
                if (isDirOpen)
                {
                    for (auto& file : dir.files)
                    {
                        ImGui::PushID(ImGui::GetID(file.name.c_str()));
                        ImGui::Indent(5.0f);
                        
                        ImGuiTreeNodeFlags fileFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_SpanFullWidth;
                        if (_currentPath == file.path) 
                        {
                            fileFlags |= ImGuiTreeNodeFlags_Selected;
                        }

                        bool isFileOpen = ImGui::TreeNodeEx(("##" + file.path.string()).c_str(), fileFlags);
                        if (ImGui::IsItemClicked())
                        {
                            _currentPath = file.path;
                            fileSelected = true;
                        }

                        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                        {
                            _currentPath = file.path;
                            ImGui::OpenPopup("##FileEdit");
                        }

                        filePopup(file);

                        ImGui::SameLine(0.0f, 0.0f);
                        if (file.icon) {
                            ImGui::Image((ImTextureID)file.icon, ImVec2(ICON_SIZE, ICON_SIZE));
                            ImGui::SameLine(0.0f, 0.0f);
                        }

                        if ((renaming && _currentPath == file.path) || file.path == _newPath)
                        {
                            fileRename(file);
                        }
                        else
                        {
                            ImGui::Text("%s", file.name.c_str());
                        }
  
                        if (isFileOpen)
                            ImGui::TreePop();

                        ImGui::PopID();
                    }
                    
                    ImGui::TreePop();
                }
                
                ImGui::PopID();
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
        ImGui::End();
    }

    void file_system_t::getSystemInfo()
    {
        try
        {
            getDirInfo(_systemInfo, _rootPath, 0);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error in getSystemInfo: " << e.what() << '\n';
        }
    }

    void file_system_t::updatePath(std::filesystem::path path)
    {
        try
        {
            if (!std::filesystem::exists(path)) {
                return;
            }

            _currentPath = path;
            uint32_t depth = 0;

            auto it = std::find_if(_systemInfo.begin(), _systemInfo.end(), 
                [&](const auto& entry)
                {
                    return entry.first == path;
                });

            if (it != _systemInfo.end())
            {
                depth = it->second.depth;   
                _systemInfo.erase(it);

                std::erase_if(_systemInfo, [&](const auto& entry) 
                {
                    return entry.second.parentPath == path;
                });
            }

            getDirInfo(_systemInfo, path, depth);
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    }

    dirInfo file_system_t::getDirInfo(std::map<std::string, dirInfo>& systemInfo, std::filesystem::path current, size_t depth, std::filesystem::path parent)
    {
        dirInfo currentDir{};
        currentDir.depth = depth;
        currentDir.dirPath = current;
        currentDir.name = current.filename().string();
        currentDir.parentPath = parent.empty() ? _rootPath : parent;

        for (const auto& entry : std::filesystem::directory_iterator(current))
        {
            if (entry.is_directory())
            {
                getDirInfo(systemInfo, entry, ++depth, current);
                continue;
            }

            currentDir.files.emplace_back(getFileInfo(entry));
        }

        systemInfo.emplace(current.string(), currentDir);
        return currentDir;
    }

    fileInfo file_system_t::getFileInfo(std::filesystem::path filePath)
    {
        fileInfo file{};
        file.path = filePath;
        file.name = filePath.filename().string();
        file.size = std::filesystem::file_size(filePath);
        file.lastModified = std::filesystem::last_write_time(filePath);
        file.extension = filePath.extension().string();

        if (isTextFileFormats(file.extension))
        {
            file.icon = _icons[SCRIPT];
        }
        else if (isImageFileFormats(file.extension))
        {
            file.icon = _icons[IMAGE];
        }
        else if (isModelFileFormats(file.extension))
        {
            file.icon = _icons[MODEL];
        }
        else
        {
            file.icon = _icons[FILE];
        }

        return file;
    }
} // namespace eng
