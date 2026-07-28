#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>




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

#include "Renderer.h"
#include "Resources/ResourceManager.h"
#include "Utilities/FileManager.h"
#include "Utilities/AssetManager.h"

#include "ModelLoader.h"
#include "Resources/Handle.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>
#include <fstream>
#include <map>
#include <memory>

#include "../UI/CinderEditor.h"
#include "UI/ImGuiPanel.h"


class SceneHierarchyPanel;
class Shader;
class Renderer;
class Scene;

class FileLoader;
class Engine
{
public:
	Engine()
	{
		g_instance = this;
	}

private:

	static Engine* g_instance;

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

	std::unique_ptr<Scene> m_currentScene{std::make_unique<Scene>()};
	std::unique_ptr<Renderer> renderer{std::make_unique<Renderer>()};
	std::unique_ptr<AssetManager> m_assetManager{std::make_unique<AssetManager>()};
	std::unique_ptr<ModelLoader> m_modelLoader{std::make_unique<ModelLoader>(m_assetManager.get())};
	std::unique_ptr<CinderEditor> m_editor{std::make_unique<CinderEditor>()};


	static std::unique_ptr<FileLoader> m_FileLoader;

	std::map<float, MeshEntity*> transparentObj{};
	Camera camera = Camera( glm::vec3(0.0f, 10.0f, -30.0f),glm::vec3(0.0f, 0.0f, 1.0f), 45.0f, static_cast<float>(scrWidth) /(scrHeight), 50.0f, 0.1, 10000);


	 void init(GLFWwindow*& window);
	// TODO: proper system into class setup
	 void sRendering();
	 void sInput();
	 void deltaTimeUpdate();

	  void imguiInit();
	 void imguiUpdate();
	void imguiRenderScene();

	 void imguiRender();

	 Handle<Model> loadModel(const std::string& file);

	void createTextures();
	void createShaders();
	void createModels();
	void createMaterials();

	void createObjectIcons();


	Entity* createFloor();

	void addMeshToScene(Model* mesh, glm::vec3 position);

	void createPointLight(const std::string& name, float radius, glm::vec3 position);

	Entity* createDirectionalLight(const std::string& name, glm::vec3 direction);


	void createCube(const std::string &name = "Cube", const char *materialName = "default", glm::vec3 position = glm::vec3(0.0f), float
	                rotationAngle = 0.0f,
	                glm::vec3 rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3 scale = glm::vec3(1.0f));



	static void keyCallback(GLFWwindow* window, int key, int action, int scancode, int mods);
	static void frameBufferSizeCallback(GLFWwindow* window, int width, int height);
	static void mouseCallback(GLFWwindow* window, double xPos, double yPos);
	static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
	static void fileDropCallback(GLFWwindow* window, int count, const char** paths);
public:
	 void run(const int w, const int h, const std::string& title);

	static Engine* get() {return g_instance;}

	float getDeltaTime() {return m_deltaTime;}

	// Events

	static Delegate<GLFWwindow*, int, int> OnWindowResized;
	static Delegate<GLFWwindow*> OnWindowClosed;

	 ~Engine();

};


