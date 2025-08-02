#include "modelloader_t.hpp"

#include <iostream>

namespace eng
{
    Assimp::Importer modelloader_t::importer;

    void modelloader_t::loadModel(const std::string& path, model_t* models)
    {
        const aiScene *scene = importer.ReadFile(path,
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices |
        aiProcess_Triangulate);

        if (nullptr == scene)
        {
            std::cout << importer.GetErrorString();
            return;
        }

        std::vector<model_t> processedModels;
        processScene(scene, processedModels);

        if (processedModels.size() > 1)
        {
            // to do: 
            // process multiple objects in one single scene
        }
        else
        {
            models[0] = processedModels.at(0);
        }
    }

    void modelloader_t::processScene(const aiScene* scene, std::vector<model_t>& models)
    {
        if (!scene->HasMeshes())
        {
            std::cout << "Scene has no meshes." << std::endl;
            return;
        }

        models.resize(scene->mNumMeshes);

        for (int32_t i = 0; i < scene->mNumMeshes; ++i)
            processMesh(scene->mMeshes[i], models[i]);
    }

    void modelloader_t::processMesh(aiMesh* mesh, model_t& model)
    {
        if (mesh->mNumVertices < 3)
        {
            std::cout << "Mesh needs atleast 3 vertices. " << std::endl;
            return;
        }

        std::vector<model_t::vertex_t> vertices;
        std::vector<model_t::index_t> indices;
        
        vertices.reserve(mesh->mNumVertices);
        indices.reserve(mesh->mNumFaces * 3); 

        processVertices(mesh, vertices, indices);

        model = model_t{vertices, indices};
    }

    void modelloader_t::processVertices(aiMesh* mesh, std::vector<model_t::vertex_t>& vertices, std::vector<model_t::index_t>& indices)
    {
        for (int32_t i = 0; i < mesh->mNumVertices; ++i)
        {
            glm::vec2 vertexUv = {0.0f, 0.0f};
            glm::vec3 vertexTangent = {0.0f, 0.0f, 0.0f};

            if (mesh->mTextureCoords[0])
            {
                vertexUv.x = mesh->mTextureCoords[0][i].x;
                vertexUv.y = 1 - mesh->mTextureCoords[0][i].y;
            }

            model_t::vertex_t vertex;
            vertex.translation= {mesh->mVertices[i].x, -mesh->mVertices[i].y, mesh->mVertices[i].z};
            if (mesh->HasVertexColors(i))
            {
                vertex.color = {mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b};
            }
            else
            {
                vertex.color = {.5f, .5f, .5f};
            }

            if (mesh->HasNormals())
            {
                glm::vec3 vertexNormal = {mesh->mNormals[i].x, -mesh->mNormals[i].y, mesh->mNormals[i].z};
                vertex.normal = vertexNormal;
            }
            if (mesh->HasTangentsAndBitangents())
            {
                vertexTangent = {mesh->mTangents[i].x, -mesh->mTangents->y, mesh->mTangents->z};
            }

            vertex.uv = vertexUv;
            vertex.tangent = vertexTangent;

            vertices.push_back(vertex);
        }

        for (int32_t i = 0; i < mesh->mNumFaces; ++i)
        {
            const aiFace &face = mesh->mFaces[i];
            if (face.mNumIndices == 3)
            {
                indices.push_back(face.mIndices[0]);
                indices.push_back(face.mIndices[1]);
                indices.push_back(face.mIndices[2]);
            }
        }
    }
} // namespace eng
