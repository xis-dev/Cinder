#pragma once

#include <GLFW/glfw3.h>

#include "Camera.h"
#include "Entity.h"

#include "Rendering/Scene.h"

#include "Renderer.h"
#include "Utilities/AssetManager.h"

#include "ModelLoader.h"
#include "Resources/Handle.h"

#include <map>
#include <memory>

#include "../UI/CinderEditor.h"
#include "src/Core/Input.h"
#include "src/Core/Window.h"
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
	std::unique_ptr<Window> m_window{};

	static bool shiftLock;

	double m_lastFrameTime{};
	double m_deltaTime{};

	std::unique_ptr<Scene> m_currentScene{std::make_unique<Scene>()};
	std::unique_ptr<Renderer> m_renderer;
	std::unique_ptr<AssetManager> m_assetManager{std::make_unique<AssetManager>()};
	// TODO: Remove and replace with FileLoader
	std::unique_ptr<ModelLoader> m_modelLoader{std::make_unique<ModelLoader>(m_assetManager.get())};
	std::unique_ptr<CinderEditor> m_editor{std::make_unique<CinderEditor>()};
	std::unique_ptr<Input> m_inputSystem;


	static std::unique_ptr<FileLoader> m_FileLoader;


	 void init(int w, int h, const std::string &title);
	// TODO: proper system into class setup
	 void sRendering();
	 void sInput();
	 void deltaTimeUpdate();

	 Handle<Model> loadModel(const std::string& file) const;

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
	void cleanup();

	static Engine* get() {return g_instance;}

	float getDeltaTime() {return m_deltaTime;}

	// Events

	static Delegate<GLFWwindow*, int, int> OnWindowResized;
	static Delegate<GLFWwindow*> OnWindowClosed;

	 ~Engine() = default;

};


