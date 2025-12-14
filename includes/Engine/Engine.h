#pragma once


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <map>

#include "Camera.h"
#include "Cube.h"
#include "Entity.h"
#include "Material.h"
#include "Mesh.h"
#include "Texture.h"
#include "Math/Vec3.h"

#include "Primitives/Plane.h"
#include "Utilities/IconRegistry.h"
#include "Scene/Scene.h"
#include <memory>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "Resources/ResourceManager.h"
#include "Utilities/FileManager.h"

#define PRINTAPI(x) std::cout << #x << std::endl;


class Shader;




namespace screen
{
	 inline int         width        {1600};
	 inline int         height       {900};
	 inline const char* title        { "GLScene" };


	 inline double lastX = width / 2;
	 inline double lastY = height / 2;

	 inline double xOffset{};
	 inline double yOffset{};
	 inline double scrollOffset{};
	 inline bool shiftLock{ true };

	 inline bool firstMouse{ true };

	 
}



class Engine
{
private:
	static GLFWwindow* m_window;

public:

	// vao used to for drawing scene addons such as the world grid and icons on entities
	unsigned addonVAO = 0;

	ResourceManager<Shader> shaders{"S_shaderObject"};
	ResourceManager<Texture> textures{"T_textureObject"};
	ResourceManager<Material> materials{"MT_materialObject"};
	ResourceManager<Mesh> meshes{"M_meshObject"};

	
	Scene m_currentScene;

	static bool cullBackface;
	static bool drawWireframe;
	double m_lastFrameTime{};
	double m_deltaTime{};
	Camera camera = Camera( glm::vec3(0.0f, 10.0f, -30.0f),glm::vec3(0.0f, 0.0f, 1.0f), 45.0f, (float)screen::width / screen::height, 15.0f);


	void init(GLFWwindow*& window);
	// TODO: proper system into class setup
	void sRendering();
	void sInput();
	void deltaTimeUpdate();

	static void imguiInit();
	void imguiUse();

	static void imguiRender();
	void renderGrid();
	void renderIcons();

	void drawAddon(int indexCount);

	void loadModel(const std::string& filePath, Material* mat);
	void processNode(aiNode* node, const aiScene* scene, const std::string& directory, Material* mat);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene, const std::string& directory, Material* mat);
	std::vector<Texture*> loadMaterialTextures(aiMaterial* mat, aiTextureType assimp_textureType, Texture::Type textureType, const std::string& directory);


	void createTextures();
	void createShaders();
	void createMeshes();			   
	void createMaterials();

	void createObjectIcons();

	void initializeAddonVAO();
	void createIconVAO();
	void createFloor();

	void addMeshToScene(Mesh* mesh, Vec3f position);

	void sendGeneralShaderUniforms();

	void createPointLight(const std::string& name, float radius, Vec3f position);

	void createDirectionalLight(const std::string& name, Vec3f direction);


	void createCube(const std::string &name = "Cube", const char *materialName = "default", Vec3f position = Vec3(0.0f), float
	                rotationAngle = 0.0f,
	                Vec3f rotationAxis = Vec3f(0.0f, 0.0f, 1.0f), Vec3f scale = Vec3f(1.0f));






	static void keyCallback(GLFWwindow* window, int key, int action, int scancode, int mods);
	static void frameBufferSizeCallback(GLFWwindow* window, int width, int height);
	static void mouseCallback(GLFWwindow* window, double xPos, double yPos);
	static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
public:
	void run();

};


