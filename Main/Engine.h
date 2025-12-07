#pragma once


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <map>

#include "Objects/Headers/Camera.h"
#include "Shapes/Headers/Cube.h"
#include "Core/Headers/Entity.h"
#include "Resources/Headers/Material.h"
#include "Resources/Headers/Mesh.h"
#include "Math/Vec3.h"

#include "Shapes/Headers/Floor.h"
#include "Core/IconRegistry.h"
#include "Main/Scene.h"
#include <memory>

#include "Resources/ResourceManager.h"

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

	ResourceManager<Shader> shaders{};
	ResourceManager<Texture> textures{};
	ResourceManager<Material> materials{};
	ResourceManager<Mesh> meshes{};

	
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

	void createTextures();
	void createShaders();
	void createMeshes();			   
	void createMaterials();

	void createObjectIcons();

	void initializeAddonVAO();
	void createIconVAO();
	void createFloor();

	void sendGeneralShaderUniforms();

	void createPointLight(const std::string& name, float radius, Vec3f position);



	void createDirectionalLight(const std::string& name, Vec3f direction);


	void createCube(const std::string &name = "Cube", const char *materialName = "default", Vec3f position = Vec3(0.0f), float
	                rotationAngle = 0.0f,
	                Vec3f rotationAxis = Vec3f(0.0f, 0.0f, 1.0f), Vec3f scale = Vec3f(1.0f));
	void loadModel(const char* path, const char* tag, const char* materialName = "default", Vec3f position = Vec3f(0.0), Vec3f scale = Vec3f(1.0), float angle = 0, Vec3f
	               rotAxis = Vec3f(0.0, 0.0, 1.0));

	static void keyCallback(GLFWwindow* window, int key, int action, int scancode, int mods);
	static void frameBufferSizeCallback(GLFWwindow* window, int width, int height);
	static void mouseCallback(GLFWwindow* window, double xPos, double yPos);
	static void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
public:
	void run();

};


