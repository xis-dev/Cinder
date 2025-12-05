#pragma once
#include <iostream>
#include <memory>

#include "Material.h"
#include "Mesh.h"
#include "Core/FileManager.h"
#include "Math/Vec3.h"
#include "Objects/Headers/Camera.h"
#include "out/build/x64-Debug/_deps/assimp-src/code/Common/StbCommon.h"
#include <assimp/Importer.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>


class Mesh;
class Material;


class Component
{

public:
   virtual ~Component() = default;
   bool active{};
   static size_t ID;

protected:

};


class Transform: public Component
{
public:
   Transform() = default;
   Transform(Vec3f position, float rotationAngle = 0.0f,
             Vec3f rotationAxis =
             Vec3f(0.0f, 0.0f, 1.0f),
             Vec3f scale = Vec3f(1.0f)): m_position(position),
                                               m_scale(scale),
                                               m_currentAngle(rotationAngle),
                                               m_currentRotationAxis(rotationAxis){}
private:
   Vec3f m_position{};
   Vec3f m_scale{1.0f};
   float m_currentAngle{};
   Vec3f m_currentRotationAxis{0.0f, 0.0f, 1.0f};

public:
   void setPosition(Vec3f position);
   Vec3f getPosition() const;

   void setScale(Vec3f scale);
   void setScale(float scalar);
   Vec3f getScale() const;
   Vec3f getRotationAxis() const;
   float getRotationAngle() const;

   void rotate(float angle, Vec3f rotationAxis);

};


class MeshRenderer: public Component
{
public:
   std::vector<std::shared_ptr<Material>> m_materials{};
   std::vector<std::shared_ptr<Mesh>> m_meshes{};
   std::vector<std::shared_ptr<Texture>> loadedTextures{};

   std::shared_ptr<Shader> m_shader{ };

public:
   MeshRenderer() = default;
   MeshRenderer(const std::shared_ptr<Material>& material, const std::shared_ptr<Mesh>& mesh)
   {
       m_materials.push_back(material);
       m_shader = Shader::getTexturedShader();
       m_meshes.push_back(mesh);
   }

    // Model loading
    // TODO: remove assimp, write obj and fbx(?) parser
    // TODO: Mesh rendering with curves(lengyel)

    // Model exists within or close to the project/working directory
   MeshRenderer(const std::shared_ptr<Shader>& shader, const char* path)
   {
       m_shader = Shader::getTexturedShader();
	   loadModel(path);
   }

    MeshRenderer(const std::shared_ptr<Material>& material, const char* path)
   {
        m_materials.push_back(material);
        m_shader = Shader::getTexturedShader();
        loadModel(path);
   }



   std::string directory{};

   void loadModel(const std::string& path)
   {
       std::cout << "Loading model from: " << path << std::endl;

		
       Assimp::Importer importer;
    //  TODO: path checking
       const aiScene* scene = importer.ReadFile(path,
           aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

       if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
       {
           std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << '\n';
           return;
       }

       directory = path.substr(0, path.find_last_of('/'));
       std::cout << "Model directory: " << directory << std::endl;
       std::cout << "Total meshes in scene: " << scene->mNumMeshes << std::endl;

       processNode(scene->mRootNode, scene);

       std::cout << "Model loading completed. Total meshes: " << m_meshes.size() << std::endl;
   }
    void processNode(const aiNode* node, const aiScene* scene)
    {
	    for (size_t i = 0; i < node->mNumMeshes; ++i)
	    {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            m_meshes.push_back((processMesh(mesh, scene)));
            m_meshes[0]->tag = "loadedModel";
	    }

        for (size_t i = 0; i < node->mNumChildren; ++i)
        {
            processNode(node->mChildren[i], scene);
        }

    }
    std::shared_ptr<Mesh> processMesh(aiMesh* mesh, const aiScene* scene)
    {
        static int meshCount = 0;
        std::vector<Vertex> vertices{};
        std::vector<unsigned int> indices{};
        std::vector<std::shared_ptr<Texture>> textures{};
        for (int i = 0; i < mesh->mNumVertices; ++i)
        {
            Vertex vert;

            auto& pos = mesh->mVertices[i];
            auto& norm = mesh->mNormals[i];
            vert.m_Position = Vec3f{ pos[0], pos[1], pos[2] };

            vert.m_Normal = Vec3f{ norm[0], norm[1], norm[2] };

            if (mesh->mTextureCoords[0])
            {
                vert.m_TexCoords = Vec2f{ mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
            }
            else
            {
                vert.m_TexCoords = Vec2f(0.0f);
            }

            vertices.push_back(vert);
        }

        for (int i = 0; i < mesh->mNumFaces; ++i)
        {
            aiFace face = mesh->mFaces[i];
            for (int j = 0; j < face.mNumIndices; ++j)
            {
                indices.push_back(face.mIndices[j]);
            } 
        }

        if (!m_shader) { m_shader = Shader::getDefaultShader();}
        auto mater = std::make_shared<Material>(m_shader);

        if (mesh->mMaterialIndex >= 0)
        {
            if (scene->mMaterials[mesh->mMaterialIndex]->GetTextureCount(aiTextureType_DIFFUSE) > 0)
            {
                std::cout << "Creating material with texture. \n";
                aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
                auto diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, Texture::Diffuse);
                textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

                auto specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, Texture::Specular);

                textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

                for (auto& tex : textures)
                {
                    mater->addTexture(tex);
                }
                m_materials.push_back(mater);
            }
            
        }
        return std::make_shared<Mesh>(vertices, indices);
    }


    std::vector<std::shared_ptr<Texture>> loadMaterialTextures(aiMaterial* material, aiTextureType type, Texture::Type texType)
    {
        std::vector<std::shared_ptr<Texture>> textures;

        for (unsigned i = 0; i < material->GetTextureCount(type); ++i)
        {
            aiString str;
            if (material->GetTexture(type, i, &str) != AI_SUCCESS) continue;
            auto fullAiStr = FileManager::getAbsolutePath(str.C_Str(), directory);
            bool skip = false;
            for (auto & loadedTexture : loadedTextures)
            {
                auto fullTexStr = FileManager::getAbsolutePath(loadedTexture->m_path);
	            if (std::strcmp(fullTexStr.c_str(), fullAiStr.c_str()) == 0)
	            {
                    textures.push_back(loadedTexture);
                    skip = true;
	                break;
	            }
            }
            if (!skip)
            {
                auto fullPath = FileManager::getAbsolutePath(str.C_Str(), directory);
                std::shared_ptr<Texture> newTexture = std::make_shared<Texture>(fullPath.c_str(), texType);
                textures.push_back(newTexture);
                loadedTextures.push_back(newTexture);
            }
        }
        return textures;
    }

    void sendShaderInput(const std::string& name, auto value)
   {
	   for (auto& mat: m_materials)
	   {
           mat->useShader();
           mat->sendShaderInput(name, value);
	   }
   }



   void useShaders() const
   {
    for (auto& mat: m_materials)
    {
        mat->useShader();
    }
   }

    void render(Camera& camera, const Transform& transform)
   {
    // TODO: remove and add back to rendering system
   	glm::mat4 worldMat{ 1.0f };
    worldMat = glm::translate(worldMat, static_cast<glm::vec3>(transform.getPosition()));
    worldMat = glm::rotate(worldMat, glm::radians((transform.getRotationAngle())), static_cast<glm::vec3>(transform.getRotationAxis())); // TODO: add personal math lib matrix operations
    worldMat = glm::scale(worldMat, static_cast<glm::vec3>(transform.getScale()));

\


    if (m_materials.empty())
    {
        m_materials.emplace_back(std::make_shared<Material>(m_shader));
    }

    for (auto& mat: m_materials)
    {
        mat->old_material_use(camera.getProjectionMatrix(), camera.getViewMatrix(), worldMat, camera.getPosition());
    }

    for (const auto& mesh : m_meshes)
    {
	    mesh->draw();
    }


    for (auto& mat : m_materials)
    {
        if (mat->m_textures.empty()) continue;
    	for (int i = 0; i < mat->m_textures.size(); i++)
        {
            Texture::unbind(GL_TEXTURE0 + i);
        }

   	}
    


   }
};

class Light: public Component
{
public:
   Light() = default;
   Light(int id, Vec3f color, float intensity): m_lightID(id), m_intensity(intensity),
                                        m_color(color){}
public:

   // TODO: give ownership to scene class
   int m_lightID{};
   int m_sceneAmount{1};
   float m_intensity{1.0f};
   Vec3f m_color{1.0f};
   virtual void use(MeshRenderer& renderer, Transform& transform){renderer.useShaders();}
};

class DirectionalLight: public Light
{
public:
   DirectionalLight() = default;
   DirectionalLight(Vec3f direction, int id, Vec3f color = Vec3f(1.0f),
                                     float intensity = 1.0f):
   Light(id, color, intensity),
   m_direction(direction)
   {

   }
   Vec3f m_direction{};
   void setDirection(Vec3f d);
   [[nodiscard]] Vec3f getDirection() const;

    void use(MeshRenderer& renderer, Transform& transform) override
    {
       Light::use(renderer, transform);
       const std::string uniformStr{"u_DirectionalLights[" + std::to_string(m_lightID) + "]."};

       renderer.sendShaderInput((uniformStr + "direction"), m_direction);
       renderer.sendShaderInput((uniformStr + "color"), m_color);
       renderer.sendShaderInput((uniformStr + "intensity"), m_intensity);
    }
};

class PointLight: public Light
{
public:
   PointLight() = default;
   PointLight(float attenuationRadius,
               int id = 0, Vec3f color = Vec3f(1.0f), float intensity = 1.0f):
   Light(id, color, intensity)
   {
       Vec3f attenuationValueSet = getAttenuationValues(attenuationRadius);
       m_constant = attenuationValueSet.x;
       m_linear = attenuationValueSet.y;
       m_quadratic = attenuationValueSet.z;

   }

   float m_constant{1.0f};
   float m_linear{1.0f};
   // Ogre3D quadratic = 1.0 / (0.01 * distance * distance)
   float m_quadratic{2.0f};

   // Radius = max light range
   // Threshold = light strength at edge
   // Ratio = linear contribution relative to quadratic at the range, >1 = more linear, <1 = more quadratic 
   Vec3f getAttenuationValues(float radius, float threshold = 0.01f, float ratio = 1.0f, float constant = 1.0f)
   {
       float reqQuotient = (1.0f / threshold) - 1.0f;

       float quadratic = reqQuotient / ((ratio + 1.0f) * radius * radius);
       float linear = ratio * quadratic * radius;

       return Vec3f(constant, linear, quadratic);
   }


   void use(MeshRenderer& renderer, Transform& transform) override
   {
      Light::use(renderer, transform);
      const std::string uniformStr{"u_PointLights[" + std::to_string(m_lightID) + "]."};

      if (m_sceneAmount < 1) return;
      renderer.sendShaderInput((uniformStr + "constant"), m_constant);
      renderer.sendShaderInput((uniformStr + "linear"), m_linear);
      renderer.sendShaderInput((uniformStr + "quadratic"), m_quadratic);

      renderer.sendShaderInput((uniformStr + "color"), m_color);
      renderer.sendShaderInput((uniformStr + "position"), transform.getPosition());
      renderer.sendShaderInput((uniformStr + "intensity"), m_intensity);
   }
};

class SpotLight: public Light
{
public:
   SpotLight() = default;
   SpotLight(Vec3f direction, float innerAngle, float outerAngle,
             int id = 0, Vec3f color = Vec3f(1.0f), float intensity = 1.0f):
   Light(id, color, intensity),
   m_direction(direction),
   // Inner and Outer cutoff values, cosine value of their respective angles
   m_innerCutoff(glm::cos(glm::radians(innerAngle))),
   m_outerCutoff(glm::cos(glm::radians(outerAngle))){}


   Vec3f m_direction{};

   float m_innerCutoff{};
   float m_outerCutoff{};

   void use(MeshRenderer& renderer, Transform& transform) override
   {
      Light::use(renderer, transform);

      const std::string uniformStr = "u_SpotLights[" + std::to_string(m_lightID) + "].";


      renderer.sendShaderInput(uniformStr + "innerCutoff", m_innerCutoff);
      renderer.sendShaderInput(uniformStr + "outerCutoff", m_outerCutoff);
      renderer.sendShaderInput(uniformStr + "color", m_color);
      renderer.sendShaderInput(uniformStr + "direction", m_direction.getNormalized());
      renderer.sendShaderInput(uniformStr + "position", transform.getPosition());
      renderer.sendShaderInput(uniformStr + "intensity", m_intensity);


   }
};


