#pragma once

#include <string>
#include <filesystem>

namespace core
{
    struct name_t
    {
        std::string name;
    };
    
    struct path_t
    {
        std::filesystem::path path;
    };
} // namespace core
