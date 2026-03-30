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
#include "Rendering/Scene.h"
#include <memory>

#include "Renderer.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "Resources/ResourceManager.h"
#include "Utilities/FileManager.h"
#include "Utilities/AssetManager.h"

#include "ModelLoader.h"

#include "Resources/Handle.h"


class Shader;
class Renderer;
class Scene;


class Engine
{
public:

private:
	// Window and mouse settings
	static GLFWwindow* m_window;
	static int scrWidth;
	static int scrHeight;
	static std::string scrTitle;

	static double xMouseOffset;
	static double yMouseOffset;
	static double scrollOffset;

	static double lastXPos;
	static double lastYPos;

	static bool shiftLock;
	static bool firstMouseInput;

	double m_lastFrameTime{};
	double m_deltaTime{};

	Scene* m_currentScene = new Scene();

	Renderer* renderer = new Renderer();

	AssetManager*  m_assetManager = new AssetManager();

	ModelLoader* m_modelLoader;

	std::map<float, MeshEntity*> transparentObj{};
	Camera camera = Camera( glm::vec3(0.0f, 10.0f, -30.0f),glm::vec3(0.0f, 0.0f, 1.0f), 45.0f, static_cast<float>(scrWidth) /(scrHeight), 15.0f);


	 void init(GLFWwindow*& window);
	// TODO: proper system into class setup
	 void sRendering();
	 void sInput();
	 void deltaTimeUpdate();

	  void imguiInit();
	 void imguiUpdate();

	 void imguiRender();

	 Model* loadModel(const std::string& file);

	void createTextures();
	void createShaders();
	void createModels();
	void createMaterials();

	void createObjectIcons();

	void createFloor();

	void addMeshToScene(Model* mesh, Vec3f position);

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
	 void run(const int w, const int h, const std::string& title);

};


