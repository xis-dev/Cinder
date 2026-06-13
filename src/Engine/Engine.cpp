#include "Engine/Engine.h"
#include <ranges>
#include "Resources/Shader.h"
#include "Resources/ResourceManager.h"
#include "Resources/Texture.h"
#include "Resources/Mesh.h"
#include "Resources/Material.h"
#include <Utilities/IconRegistry.h>
#include "Resources/Model.h"

#include "Entity/LightEntity.h"
#include "Entity/MeshEntity.h"

#include "Rendering/Renderer.h"



#define GLM_ENABLE_EXPERIMENTAL
#include "FileLoader.h"
#include "glm/gtx/norm.hpp"
GLFWwindow* Engine::m_window{};

std::unique_ptr<FileLoader> Engine::m_FileLoader{};

double Engine::xMouseOffset{};
double Engine::yMouseOffset{};
double Engine::scrollOffset{};

int Engine::scrWidth = 800;
int Engine::scrHeight = 600;
std::string Engine::scrTitle = "Cinder";

double Engine::lastXPos = 0.0;
double Engine::lastYPos = 0.0;


bool Engine::firstMouseInput{true};
bool Engine::shiftLock{true};


glm::vec3 cubePositions[] = {
	glm::vec3(0.0f, 1.0f,  0.0f),
	glm::vec3(2.0f, 1.0f, -5.0f),
	glm::vec3(-2.0f, 1.0f, -5.0f),
	glm::vec3(4.0f, 1.0f, -10.0f),
	glm::vec3(-4.0f, 1.0f, -10.0f),

	// FLOATING cubes
	glm::vec3(2.0f,  2.0f, -6.0f),
	glm::vec3(-2.0f,  3.0f, -8.0f),
	glm::vec3(1.5f,  1.5f, -3.0f),
	glm::vec3(-1.5f,  2.5f, -4.0f),
	glm::vec3(0.0f,  4.0f, -7.0f)
};

glm::vec3 rotationAxes[] = {
	glm::vec3(0.0f, 1.0f, 0.0f), // upright (Y axis)
	glm::vec3(0.0f, 1.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f),

	// tilted ones
	glm::vec3(1.0f, 0.0f, 0.0f), // X tilt
	glm::vec3(0.0f, 0.0f, 1.0f), // Z tilt
	glm::vec3(1.0f, 1.0f, 0.0f), // diagonal
	glm::vec3(0.0f, 1.0f, 1.0f),
	glm::vec3(1.0f, 0.0f, 1.0f)
};


glm::vec3 pointLightPositions[] = {
	glm::vec3( 0.7f,  2.0f,  2.0f),
	glm::vec3( 2.3f, -3.3f, -4.0f),
	glm::vec3(-4.0f,  2.0f, -12.0f),
	glm::vec3( 0.0f,  0.0f, -3.0f),
	glm::vec3(4.0f,  2.0f, -2.0f),
	glm::vec3(-4.0f,  24.0f, 12.0f),
	glm::vec3(14.0f,  -2.0f, -12.0f),
	glm::vec3(0.0f,  0.0f, 2.0f),
	glm::vec3(-15.0f,  -5.0f, 0.0f),	glm::vec3( 0.7f,  2.0f,  2.0f),
	glm::vec3( 2.3f, -3.3f, -4.0f),
	glm::vec3(-4.0f,  2.0f, -12.0f),
	glm::vec3( 0.0f,  0.0f, -3.0f),
	glm::vec3(4.0f,  2.0f, -2.0f),
	glm::vec3(-4.0f,  24.0f, 12.0f),
	glm::vec3(14.0f,  -2.0f, -12.0f),
	glm::vec3(0.0f,  0.0f, 2.0f),
	glm::vec3(-15.0f,  -5.0f, 0.0f)

};
void Engine::run(const int w, const int h, const std::string& title)
{
	// initialize the engine, main loop

	scrWidth = w;
	scrHeight = h;
	scrTitle = title;

	lastXPos = scrWidth/2.0;
	lastYPos = scrHeight/2.0;

	// TODO: replace and construct better with systems
	init(m_window);
	while (!glfwWindowShouldClose(m_window))
	{
		glfwPollEvents();

		deltaTimeUpdate();

		imguiUpdate();
		sInput();
		sRendering();

		imguiRender();

		glfwSwapBuffers(m_window);
	}

	// shutdown imgui and terminate glfw
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
}

Engine::~Engine()
{
	delete renderer;
	delete m_assetManager;
	delete m_currentScene;
	delete m_modelLoader;
}


void Engine::init(GLFWwindow*& window)
{
	
	// initialize glfw and give window hints for the version and the opengl profile we want to use
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create the glfw window
	window = glfwCreateWindow(scrWidth, scrHeight, scrTitle.c_str(), nullptr, nullptr);

	// error handing glfw window
	if (!window)
	{
		std::cout << "FAILED TO INITIALIZE GLFW WINDOW. \n";
		glfwTerminate();
		return;
	}

	// tell glfw what window we are using, self-ex
	glfwMakeContextCurrent(window);

	// load glad + error handing
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "FAILED TO INITIALIZE GLAD. \n";
	}
	m_modelLoader = new ModelLoader(m_assetManager);

	glfwSwapInterval(0);
	// set callbacks, input mode and enable depth
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// Backface Culling
	/*glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);*/
	glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);
	glfwSetScrollCallback         (window, Engine::scrollCallback);
	glfwSetCursorPosCallback      (window, Engine::mouseCallback);
	glfwSetKeyCallback            (window, Engine::keyCallback);
	glfwSetDropCallback		      (window, Engine::fileDropCallback);
	glfwSetInputMode              (window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	Cube::computeTangents();
	Plane::computeTangents();
	imguiInit();

	renderer->init(m_window, m_assetManager, m_currentScene, &scrWidth, &scrHeight);


	// as is obvious, materials must be created after shaders and textures
	createTextures();
	createShaders();
	createMaterials();
	createModels();

	createObjectIcons();


	auto robot = loadModel("C:/Users/PC/Desktop/dev/C++/Cinder/assets/Models/matikantenhauser/scene.gltf");
	auto robotEnt = m_currentScene->createEntity<MeshEntity>("Robot", m_assetManager->models.get(robot));
	//robotEnt->setRotation(glm::vec3(1.0f, 0.0f, 0.0f), -90.0f);
	robotEnt->setScale(15.0f);

	createFloor();

	//loadModel("c:/users/pc/desktop/c++/glscene/models/Chest_LowPoly.obj", "Chest", "default", glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(5.0f));

	//
	// for (int i = 1; i < 10; ++i)
	// {
	// 	createCube("cube", "container",
	// 		cubePositions[i], 20.0f * i,
	// 		glm::normalize(rotationAxes[i]),
	// 		glm::vec3(2.0f));
	// }

	createDirectionalLight("DirectionalLight", glm::vec3(3.0f, -10.0f, 3.0f));


	for (int i = 0; i < 1;++i) {
	createPointLight("PointLight" + std::to_string(i), 500.0f ,glm::vec3(pointLightPositions[i].x, pointLightPositions[i].y, pointLightPositions[i].z));

	}

	//auto* cube = m_currentScene->createEntity<MeshEntity>("Cube", m_assetManager->models.get("cube"));

	


}

void Engine::sRendering()
{
	renderer->render(camera);
}


void Engine::sInput()
{

	camera.processMouseInput(m_window, static_cast<float>(xMouseOffset), static_cast<float>(yMouseOffset), static_cast<float>(scrollOffset));
	camera.processKeyboardInput(m_window, static_cast<float>(m_deltaTime));

	xMouseOffset = 0.0f;
	yMouseOffset = 0.0f;
	scrollOffset = 0.0f;
}

void Engine::deltaTimeUpdate()
{
	auto currentTime = static_cast<float>(glfwGetTime());
	m_deltaTime = currentTime - m_lastFrameTime;
	m_lastFrameTime = currentTime;
}


 
void Engine::imguiInit()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(m_window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
	ImGui_ImplOpenGL3_Init();
}

void Engine::imguiUpdate()
{
	// setup imgui for a new frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// initialize and assign entity names for imgui dropdown
	std::vector<const char*> entityNames{};
	entityNames.reserve(m_currentScene->getEntities().size());
	for (auto& e : m_currentScene->getEntities())
	{
		entityNames.push_back(e->getTag());
	}

	auto matNames = m_assetManager->materials.getNames();
	std::vector<const char*> matNamesPtr{};
	matNamesPtr.reserve(matNames.size());

	for (auto& s : matNames)
	{
		matNamesPtr.push_back(s.first.c_str());
	}

	auto meshNames = m_assetManager->models.getNames();
	std::vector<const char*> meshNamesPtr{};
	meshNamesPtr.reserve(meshNames.size());

	for (auto& s : meshNames)
	{
		meshNamesPtr.push_back(s.first.c_str());
	}

	

	ImGui::Begin("Linking ts broke my ass");

	if (ImGui::BeginTabBar("Content"))
	{
		if (ImGui::BeginTabItem("Entities"))
		{
			static int entityIndex{};
			ImGui::Combo("Entities", &entityIndex, entityNames.data(), static_cast<int>(entityNames.size()));
			if (entityIndex >= 0 && !m_currentScene->getEntities().empty())
			{
				auto& currentEntity = m_currentScene->getEntities()[entityIndex];
				currentEntity->imguiDraw();
			}


			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Materials"))
		{
			static int materialIndex{};
			ImGui::Combo("Materials", &materialIndex, matNamesPtr.data(), static_cast<int>(matNamesPtr.size()));
			if (materialIndex >= 0)
			{
				if (Material* currentMaterial = m_assetManager->materials.get(m_assetManager->materials.getHandle(matNamesPtr[materialIndex])))
				{
					static float materialColor[3]{};
					static float ambientStr{};
					static float diffuseStr{};
					static float specularStr{};
					static float shininess{};

					ambientStr = currentMaterial->getAmbience();
					diffuseStr = currentMaterial->getDiffuse();
					specularStr = currentMaterial->getSpecular();
					shininess = currentMaterial->getShininess();

					for (int index = 0; index <= 2; ++index)
					{
						materialColor[index] = currentMaterial->getColor()[index];
					}

					ImGui::DragFloat("Ambience", &ambientStr);
					ImGui::ColorEdit3("Colour", &materialColor[0]);
					ImGui::DragFloat("Diffuse", &diffuseStr);
					ImGui::DragFloat("Specular", &specularStr);
					ImGui::DragFloat("Shininess", &shininess);

					currentMaterial->setColor(glm::vec3(materialColor[0], materialColor[1], materialColor[2]));
					currentMaterial->setDiffuse(diffuseStr);
					currentMaterial->setAmbience(ambientStr);
					currentMaterial->setSpecular(specularStr);
					currentMaterial->setShininess(shininess);
				}
				
			}
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Meshes"))
		{
			static int meshIndex{};
			ImGui::Combo("Meshes", &meshIndex, meshNamesPtr.data(), meshNamesPtr.size());
			if (meshIndex >= 0)
			{
				if (Model* currentMesh = m_assetManager->models.get(m_assetManager->models.getHandle(meshNamesPtr[meshIndex])))
				{
					static float posToAdd[3]{};
					ImGui::DragFloat3("Position to add Mesh", posToAdd);
					if (ImGui::Button("Add Mesh", ImVec2(100, 50)))
					{
						addMeshToScene(currentMesh, glm::vec3(posToAdd[0], posToAdd[1], posToAdd[2]));
					}

				}
			}

			ImGui::EndTabItem();
		}


		ImGui::EndTabBar();

	}

	ImGui::DragFloat("SSAO Strength", &renderer->ssaoStr, 0.1f);
	ImGui::DragFloat("Gamma Correction exp", &renderer->gamma, 0.1f);
	ImGui::DragFloat("Parallax Map Height", &renderer->parallaxScale, 0.1f);
	ImGui::DragFloat("HDR Exposure", &renderer->hdrExposure, 0.1f);
	ImGui::Checkbox("SSAO", &renderer->useSSAO);
	ImGui::Checkbox("Grid", &renderer->drawGrid);
	ImGui::Checkbox("HDR", &renderer->hdr);
	ImGui::Checkbox("Blinn-Phong", &renderer->blinnLighting);
	ImGui::Checkbox("Draw Cubemap", &renderer->cubeMapEnabled);
	ImGui::Checkbox("Enable Backface Culling", &renderer->cullBackface);
	ImGui::Checkbox("Draw Wireframe", &renderer->drawWireframe);
	std::string deltaTimeText{};
	std::string frameRateText{};
	deltaTimeText = "Delta Time: " + std::to_string(static_cast<int>(std::floor(float(1000 / (1 / m_deltaTime))))) + " ms/s";
	frameRateText = "FPS: " + std::to_string(static_cast<int>(std::floor((float(1 / m_deltaTime)))));
	ImGui::Text(deltaTimeText.c_str());
	ImGui::Text(frameRateText.c_str());

	// Entity Tab

	ImGui::End();
}


void Engine::imguiRender()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


Handle<Model> Engine::loadModel(const std::string& file)
{
	const auto fullPath = FileManager::getPath(file);
	if (!FileManager::fileExists(fullPath))
	{
		std::cerr << "ENGINE:: Cannot load this model.\n";
		return Handle<Model>{0};
	}
	return m_modelLoader->loadModel(file);
}


void Engine::createTextures()

{
	m_assetManager->textures.add(Texture("assets/Textures/empty.jpg"), "default");
	m_assetManager->textures.add(Texture("assets/Textures/aphex.gif"), "aphex");
	auto floorTex = m_assetManager->textures.add(Texture("assets/Textures/bricks2.jpg", Texture::Diffuse, true, GL_REPEAT), "floor");
	m_assetManager->textures.add(Texture("assets/Textures/bricks2_normal.jpg", Texture::Normal, true, GL_REPEAT), "floor_normal");
	m_assetManager->textures.add(Texture("assets/Textures/bricks2_disp.jpg", Texture::Height, true, GL_REPEAT), "floor_disp");
	m_assetManager->textures.add(Texture("assets/Textures/container_diffuse.png"), "container");
}


void Engine::createShaders()
{
	m_assetManager->shaders.add(Shader("assets/Shaders/default.vert", "assets/Shaders/default_lit.frag"), "lit");
	m_assetManager->shaders.add(Shader("assets/Shaders/default.vert", "assets/Shaders/default_unlit.frag"), "unlit");
	m_assetManager->shaders.add(Shader("assets/Shaders/default.vert", "assets/Shaders/deferred/deferred.frag"), "textured_lit");
	m_assetManager->shaders.add(Shader("assets/Shaders/world_grid.vert", "assets/Shaders/world_grid.frag"), "grid");
	m_assetManager->shaders.add(Shader("assets/Shaders/item_icon.vert", "assets/Shaders/item_icon.frag"), "icon");
	m_assetManager->shaders.add(Shader("assets/Shaders/default.vert", "assets/Shaders/singleColor.frag"), "border");
	m_assetManager->shaders.add(Shader("assets/Shaders/screen.vert", "assets/Shaders/screen.frag"), "screenShader");
	m_assetManager->shaders.add(Shader("assets/Shaders/shadow/shadowMap.vert", "assets/Shaders/shadow/shadowMap.frag"), "shadowMap");

	m_assetManager->shaders.add(Shader("assets/Shaders/shadow/pointMap.vert", "assets/Shaders/shadow/pointMap.frag", "assets/Shaders/shadow/pointMap.geom"), "pointMap");
	m_assetManager->shaders.add(Shader("assets/Shaders/screen.vert", "assets/Shaders/hdr.frag"), "HDR");
	m_assetManager->shaders.add(Shader("assets/Shaders/screen.vert", "assets/Shaders/bloomBlur.frag"), "bloomBlur");
	m_assetManager->shaders.add(Shader("assets/Shaders/screen.vert", "assets/Shaders/bloom.frag"), "bloom");

	m_assetManager->shaders.add(Shader("assets/Shaders/default.vert", "assets/Shaders/deferred/deferred.frag"), "deferredShader");
	m_assetManager->shaders.add(Shader("assets/Shaders/screen.vert", "assets/Shaders/deferred/lightPass.frag"), "deferredLightPass");

	m_assetManager->shaders.add(Shader("assets/Shaders/screen.vert", "assets/Shaders/ssao/ssao.frag"), "ssaoShader");
	m_assetManager->shaders.add(Shader("assets/Shaders/screen.vert", "assets/Shaders/ssao/ssaoBlur.frag"), "ssaoBlur");


}

void Engine::createModels()
{
	Handle<Material> container = m_assetManager->materials.getHandle("container");
	auto* cont = m_assetManager->materials.get(container);
	cont->setColor(1.0f, 1.0f, 1.0f);
	Handle<Material> def = m_assetManager->materials.getHandle("default");
	Handle<Material> floor = m_assetManager->materials.getHandle("floor");
	m_assetManager->models.add(Model(ModelSet{std::move(Mesh(Cube::vertices, Cube::indices)), container}), "cube");
	m_assetManager->models.add(Model(ModelSet{ std::move(Mesh(Plane::vertices, Plane::indices)), def}), "plane");
	m_assetManager->models.add(Model(ModelSet{ std::move(Mesh(Plane::vertices, Plane::indices)), floor }), "floor");
}

void Engine::createMaterials()
{
	m_assetManager->materials.add(Material(m_assetManager->shaders.getHandle("lit")), "default");

	m_assetManager->materials.add(Material(m_assetManager->shaders.getHandle("textured_lit")), "lit");

	m_assetManager->materials.add(Material(m_assetManager->shaders.getHandle("unlit")), "unlit");

	m_assetManager->materials.add(Material(m_assetManager->shaders.getHandle("textured_lit"),
		m_assetManager->textures.getHandle("aphex")), "aphex");

	Handle<Material> floorMat = m_assetManager->materials.add(Material(m_assetManager->shaders.getHandle("textured_lit"), m_assetManager->textures.getHandle("floor")), "floor");
	m_assetManager->materials.get(floorMat)->setShininess(16.0f);
	m_assetManager->materials.get(floorMat)->setSpecular(0.15f);
	m_assetManager->materials.get(floorMat)->addTexture(m_assetManager->textures.getHandle("floor_normal"));
	m_assetManager->materials.get(floorMat)->addTexture(m_assetManager->textures.getHandle("floor_disp"));

	Handle<Texture> containerTex = m_assetManager->textures.getHandle("container");

	auto containerHandle = m_assetManager->materials.add(Material(m_assetManager->shaders.getHandle("textured_lit"), containerTex) , "container");

	m_assetManager->materials.get(containerHandle)->addTexture(m_assetManager->textures.add(Texture("assets/Textures/container_specular.png", Texture::Type::Specular), "container_specular"));

}

void Engine::createObjectIcons()
{
	Handle<Texture> directionalLightIcon = m_assetManager->textures.add(Texture("assets/Textures/light-icon.png"), "icon_directionalLight");
	Handle<Texture> pointLightIcon = m_assetManager->textures.add(Texture("assets/Textures/light-icon.png"), "icon_pointLight");
	Handle<Texture> spotLightIcon = m_assetManager->textures.add(Texture("assets/Textures/light-icon.png"), "icon_spotLight");

	IconRegistry::registerType<DirectionalLight>(m_assetManager->textures.get(directionalLightIcon));
	IconRegistry::registerType<PointLight>(m_assetManager->textures.get(pointLightIcon));
	IconRegistry::registerType<SpotLight>(m_assetManager->textures.get(spotLightIcon));

}



void Engine::createFloor()
{
	auto* floor = m_currentScene->createEntity<MeshEntity>("Floor", m_assetManager->models.get("floor"));
	floor->setScale(35.0f);
	//for (auto& modelSet : floor->getModel()->getMeshes())
	//{
	//	auto* mater = m_assetManager->materials.get(modelSet.mat);
	//	mater->setColor(10.0f, 5.0f, 15.0f);
	//}
}

void Engine::addMeshToScene(Model* model, glm::vec3 position)
{
	auto* entity = m_currentScene->createEntity<MeshEntity>("NewObject", model);
	entity->setPosition(position);
}




void Engine::createPointLight(const std::string& name, float radius, glm::vec3 position)
{
	auto* light = m_currentScene->createEntity<PointLight>(name, radius);
	light->setPosition(position);
}

void Engine::createDirectionalLight(const std::string& name, glm::vec3 direction)
{
	auto* light = m_currentScene->createEntity<DirectionalLight>(name, direction);
	light->setPosition(3.0f);
	light->setIntensity(0.1f);
}

void Engine::createCube(const std::string& name, const char* materialName, glm::vec3 position, float rotationAngle, glm::vec3 rotationAxis,
                        glm::vec3 scale)
{
	auto cube = m_currentScene->createEntity<MeshEntity>("Cube", m_assetManager->models.get("cube"));
	cube->setPosition(position);
	cube->setRotation(rotationAxis, rotationAngle);
	cube->setScale(scale);
}





void Engine::keyCallback(GLFWwindow* window, int key, int action, int scancode, int mods)
{

	if (glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		shiftLock = !shiftLock;
		if (shiftLock)
		{
			firstMouseInput = true;
			glfwSetCursorPos(m_window, static_cast<float>(scrWidth) / 2.0f, static_cast<float>(scrHeight) / 2.0f);
		}
		glfwSetInputMode(m_window, GLFW_CURSOR, shiftLock ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
	}
}

void Engine::frameBufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);

	scrWidth = width;
	scrHeight = height;
}

void Engine::mouseCallback(GLFWwindow* window, double xPos, double yPos)
{
	if (!shiftLock) return;
	if (firstMouseInput)
	{
		lastXPos = xPos;
		lastYPos = yPos;
		firstMouseInput = false;
	}

	xMouseOffset = xPos - lastXPos;
	yMouseOffset = lastYPos - yPos;

	lastXPos = xPos;
	lastYPos = yPos;
}

void Engine::scrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
	scrollOffset = yOffset;
}

void Engine::fileDropCallback(GLFWwindow *window, int count, const char **paths)
{
	for (int i = 0; i < count; ++i)
	{

		if (m_FileLoader->loadFile(paths[i]))
		{
		}

	}

}




