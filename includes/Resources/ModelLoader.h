#pragma once
#include "Model.h"
#include "Mesh.h"
#include "Texture.h"

#include "assimp/material.h"
#include <vector>

#include "Material.h"

#include "Handle.h"

struct AssetManager;
class Texture;

struct aiNode;
struct aiScene;
struct aiMesh;
class ModelLoader
{
public:
    ModelLoader(AssetManager* manager): ASSET_MANAGER(manager){}

    AssetManager* ASSET_MANAGER;
    Handle<Model> loadModel(const std::string& file);
    void processMesh(const aiMatrix4x4 &transformMat, Model &modelToLoadInto, aiMesh *mesh, const aiScene *scene, const std::string &
                     directory);
    void processNode(Model& modelToLoadInto, aiNode* node, const aiScene* scene, const std::string& directory);
    std::vector<Handle<Texture>> loadMaterialTextures(aiMaterial *mat, aiTextureType
                                                      assimp_textureType, Texture::Type
                                                      textureType, const std::string &directory);

private:
    std::unordered_map<unsigned, Handle<Material>> loadedMaterials; // Material handles with assimp scene indexes to get already loaded materials
    std::unordered_map<std::string, Handle<Texture>> loadedTextures; // Texture handles with their full path locations to get already loaded textures
};