#include "ModelLoader.h"

#include "FileManager.h"
#include "AssetManager.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"




Model* ModelLoader::loadModel(const std::string &file)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ENGINE::ASSIMP:: " << importer.GetErrorString() << std::endl;
    	return nullptr;
    }

	std::filesystem::path p(file);
	std::string directory = p.parent_path().string();

    Model tempModel{};
    processNode(tempModel, scene->mRootNode, scene, directory);
	return ASSET_MANAGER->models.get(ASSET_MANAGER->models.add(std::move(tempModel), "empty"));
}

void ModelLoader::processMesh(Model &modelToLoadInto, aiMesh *mesh, const aiScene *scene, const std::string &directory)
{
    std::vector<Vertex> vertices;
	std::vector<unsigned> indices;
	std::vector<Handle<Texture>> ts;
	for (unsigned i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		glm::vec3 vector;

		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;

		vertex.position = Vec3f(vector.x, vector.y, vector.z);

		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;

		vertex.normal = Vec3f(vector.x, vector.y, vector.z);

		if (mesh->mTextureCoords[0])
		{
			glm::vec2 vec;

			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;

			vertex.texCoords = Vec2f(vec.x, vec.y);
		}
		else
		{
			vertex.texCoords = Vec2f(0.0f, 0.0f);
		}

		vertices.push_back(vertex);
	}

	for (unsigned i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	if (mesh->mMaterialIndex >= 0)
	{
		Material tempMat{ASSET_MANAGER->shaders.getHandle("lit")};
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Handle<Texture>> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, Texture::Diffuse, directory);

		ts.insert(ts.end(), diffuseMaps.begin(), diffuseMaps.end());
		std::vector<Handle<Texture>> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, Texture::Specular, directory);

		ts.insert(ts.end(), specularMaps.begin(), specularMaps.end());

		for (auto tex : ts)
		{
			tempMat.addTexture(tex);
		}
		// TODO: Mat name may create problems

		modelToLoadInto.add(ModelSet{ Mesh(vertices, indices), ASSET_MANAGER->materials.add(std::move(tempMat), mesh->mName.C_Str()) });
	}

}



void ModelLoader::processNode(Model &modelToLoadInto, aiNode *node, const aiScene *scene, const std::string &directory)
{
    for (unsigned i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        (processMesh(modelToLoadInto, mesh, scene, directory));
    }

    for (unsigned i = 0; i < node->mNumChildren; i++)
    {
        processNode(modelToLoadInto, node->mChildren[i], scene, directory);
    }
}

std::vector<Handle<Texture>> ModelLoader::loadMaterialTextures(aiMaterial *mat, aiTextureType assimp_textureType,
	Texture::Type textureType, const std::string &directory)
{
	std::vector<Handle<Texture>> ts;

	for (unsigned i = 0; i < mat->GetTextureCount(assimp_textureType); i++)
	{
		aiString str;
		mat->GetTexture(assimp_textureType, i, &str);
		std::filesystem::path texturePath = std::filesystem::path(str.C_Str());
		if (!texturePath.is_absolute())
		{
			texturePath = std::filesystem::path(directory) / texturePath;
		}
		auto texStr = std::filesystem::path(str.C_Str());
		bool skip = false;

		auto& allTextures = ASSET_MANAGER->textures.getAllResources();
		auto allNames = ASSET_MANAGER->textures.getNames();
		for (auto& r : allNames)
		{
			if (std::strcmp(ASSET_MANAGER->textures.get(r.first)->getLocation().c_str(), texturePath.string().c_str()) == 0)
			{
				ts.push_back(ASSET_MANAGER->textures.getHandle(r.first));
				skip = true;
				break;
			}
		}
		if (!skip)
		{
			auto slash = texturePath.string().find_last_of("/\\");
			auto dot = texturePath.string().find_last_of('.');
			std::string textureName = texturePath.string().substr(slash == std::string::npos ? 0 : slash + 1,
				dot - (slash == std::string::npos ? 0 : slash + 1));
			Handle<Texture> tex = ASSET_MANAGER->textures.add(Texture(texturePath.string(), textureType), textureName);
			ts.push_back(tex);
		}
	}
	return ts;
}
