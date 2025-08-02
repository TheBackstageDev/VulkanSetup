#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "model_t.hpp"
#include "vk/vk_device.hpp"

#include <string>

namespace eng
{
    using namespace eng;

    class modelloader_t
    {
    public:
        static void loadModel(const std::string& path, model_t* models);
    private:
        static Assimp::Importer importer;

        static void processScene(const aiScene* scene, std::vector<model_t>& models);
        static void processMesh(aiMesh* mesh, model_t& model);
        static void processVertices(aiMesh* mesh, std::vector<model_t::vertex_t>& vertices, std::vector<model_t::index_t>& indices);
    };
} // namespace eng
