#include "ModelLoader.h"

#include "FileManager.h"
#include "AssetManager.h"
#include "DefaultMacros.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"




Handle<Model> ModelLoader::loadModel(const std::string &file)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace | aiProcess_GlobalScale);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ENGINE::ASSIMP:: " << importer.GetErrorString() << std::endl;
    	return Handle<Model>{0};
    }

	std::filesystem::path p(file);
	std::string directory = p.parent_path().string();

    Model tempModel{};
	p = std::filesystem::canonical(p);
	auto slash = p.string().find_last_of("/\\");
	auto dot = p.string().find_last_of('.');
	std::string modelName = p.string().substr(slash == std::string::npos ? 0 : slash + 1,
		dot - (slash == std::string::npos ? 0 : slash + 1));
    processNode(tempModel, scene->mRootNode, scene, directory);
	loadedMaterials.clear();
	loadedTextures.clear();
	return (ASSET_MANAGER->models.add(std::move(tempModel), modelName));
}

void ModelLoader::processMesh(const aiMatrix4x4& transformMat, Model& modelToLoadInto, aiMesh *mesh, const aiScene *scene, const std::string &directory)
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

		aiVector3D aiVert{vector.x, vector.y, vector.z};
		aiVert = transformMat * aiVert;
		vertex.position = glm::vec3(aiVert.x, aiVert.y, aiVert.z);

		if (mesh->mNormals)
		{
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
		}
		else
		{
			vector = {0.0, 0.0, 1.0};
		}

		vertex.normal = glm::vec3(vector.x, vector.y, vector.z);

		if (mesh->HasTangentsAndBitangents())
		{
			vertex.tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
			// assimp loads into a right handed system, opengl uses left handed
			vertex.bitangent = glm::vec3(-mesh->mBitangents[i].x, -mesh->mBitangents[i].y, -mesh->mBitangents[i].z);
		}
		else
		{
			vertex.tangent = glm::vec3(1.0f, 0.0f, 0.0f);
			vertex.bitangent = glm::vec3(0.0f, 1.0f, 0.0f);
		}

		if (mesh->mTextureCoords[0])
		{
			glm::vec2 vec;

			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.texCoords = glm::vec2(vec.x, vec.y);
		}
		else
		{
			vertex.texCoords = glm::vec2(0.0f, 0.0f);
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
	// Default textured shader material
	Material tempMat{ASSET_MANAGER->shaders.getHandle(SHADER_DEFAULT_TEXTURED_LIT)};
	// Default 0 index means we use the empty default material
	// Ensure we're accessing a valid material index

	// Get assimp material
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	if (!material)
	{
		std::cerr << "ASSIMP:: Null material on mesh: " << mesh->mName.C_Str() << std::endl;
		return;
	}
	const char* matName = material->GetName().C_Str();

	// Has this material already been loaded?
	if (loadedMaterials.contains(mesh->mMaterialIndex))
	{
		modelToLoadInto.add((ModelSet{Mesh(vertices, indices), loadedMaterials[mesh->mMaterialIndex]}));
		return;
	}

	std::vector<Handle<Texture>> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, Texture::Diffuse, directory);

	ts.insert(ts.end(), diffuseMaps.begin(), diffuseMaps.end());

	std::vector<Handle<Texture>> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, Texture::Specular, directory);

	ts.insert(ts.end(), specularMaps.begin(), specularMaps.end());

	std::vector<Handle<Texture>> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, Texture::Normal, directory);

	ts.insert(ts.end(), normalMaps.begin(), normalMaps.end());

	for (auto tex : ts)
	{
		tempMat.addTexture(tex);
	}

	aiColor3D diffuseColor;
	material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
	tempMat.setColor({diffuseColor.r, diffuseColor.g, diffuseColor.b});

	Handle<Material> matHandle = ASSET_MANAGER->materials.add(std::move(tempMat), matName);
	loadedMaterials.insert({mesh->mMaterialIndex, matHandle});
	modelToLoadInto.add((ModelSet{ std::move(Mesh(vertices, indices)), matHandle}));

}



void ModelLoader::processNode(Model& modelToLoadInto, aiNode *node, const aiScene *scene, const std::string &directory)
{
    for (unsigned i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    	// aiMatrix4x4 a{1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0};
    	// if (node->mTransformation == a)
    	// {
    	// 	std::cout << "No\n";
    	// }
        (processMesh(node->mTransformation, modelToLoadInto, mesh, scene, directory));
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
			texturePath = std::filesystem::canonical(texturePath);
		}
		auto texStr = std::filesystem::path(str.C_Str());
		bool skip = false;

		auto& allTextures = ASSET_MANAGER->textures.getAllResources();
		auto allNames = ASSET_MANAGER->textures.getNames();
		for (auto& [loadedTexPath, loadedTexHandle] : loadedTextures)
		{
			if (std::strcmp(loadedTexPath.c_str(), texturePath.string().c_str()) == 0) // Compare loaded texture path with current texture path
			{
				ts.push_back(loadedTexHandle);
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
			Handle<Texture> tex = ASSET_MANAGER->textures.add(Texture(texturePath.string(), textureType, true), textureName);
			loadedTextures.insert({texturePath.string(), tex});
			ts.push_back(tex);
		}
	}
	return ts;
}
