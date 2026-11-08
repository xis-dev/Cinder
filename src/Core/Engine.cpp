#include "Core/Engine.h"

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

#include "FileManager.h"

#include "Cube.h"
#include "Plane.h"

#include "DefaultMacros.h"

#include "Handle.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "FileLoader.h"
#include "glm/gtx/norm.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Components/MeshComponent.h"
#include "Components/LightComponent.h"

#include <variant>

Engine* Engine::g_instance {nullptr};

Delegate<GLFWwindow*, int, int> Engine::OnWindowResized{};
Delegate<GLFWwindow*> Engine::OnWindowClosed{};



std::unique_ptr<FileLoader> Engine::m_FileLoader{};

bool Engine::shiftLock{true};


void Engine::run(const int w, const int h, const std::string& title)
{
	// initialize the engine, main loop
	init(w, h, title);

	while (!m_window->shouldClose())
	{

		deltaTimeUpdate();
		m_inputSystem->update();
		m_renderer->update();
		m_editor->update();
		m_currentScene->end();
	}

	cleanup();
}

void Engine::cleanup() {

	m_editor->destroy();
	Window::CleanupBackend();

}


void Engine::init(int w, int h, const std::string& title)
{

	// Initialize backend for all windows
	Window::InitializeBackend();

	 m_window = Window::Create(w, h, title);
	if (!m_window) {
		std::cout << "ENGINE: Failed to initialize main window.\n";
		Window::CleanupBackend();
		return;
	}

	m_inputSystem = std::make_unique<Input>(m_window.get());

	m_currentScene->init(m_assetManager.get(),
		Camera(glm::vec3(0.0f, 1.0f, -30.0f),
					glm::vec3(0.0f, 0.0f, 1.0f),
					45.0f,
					static_cast<float>(m_window->getWindowSize().first) / static_cast<float>(m_window->getWindowSize().second),
					50.0f, 1.0f, 2000.0f
		));

	m_renderer = std::make_unique<Renderer>(m_window.get(), m_assetManager.get(), m_currentScene.get());

	m_editor->initialize(m_window.get(), m_currentScene.get(), m_renderer.get());

	// TODO: Rempve and place in correct places
	glfwSetDropCallback		      (m_window->getGlfwWindow(), Engine::fileDropCallback);
	//glfwSetInputMode              (m_window->getGlfwWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	Cube::computeTangents();
	Plane::computeTangents();


	// as is obvious, materials must be created after shaders and textures
	createTextures();
	createShaders();
	createMaterials();
	createModels();

	createObjectIcons();

	m_renderer->createPasses();
	// Init renderer with 1,1 size, will take upon the size of render panel in user interface

	auto sponza = loadModel("assets/Models/sponza_palace/scene.gltf");
	auto sponzaEnt = m_currentScene->createEntity("Sponza Palace");
	sponzaEnt->addComponent<MeshComponent>(m_assetManager->models.get(sponza));

	//robotEnt->setRotation(glm::vec3(1.0f, 0.0f, 0.0f), -90.0f);
	//sponzaEnt->setScale(15.0f);

	auto floorEnt = createFloor();
	auto dirEnt = createDirectionalLight("DirectionalLight", glm::vec3(-1.0f, -1.0f, -1.0f));

	dirEnt->setParent(*sponzaEnt);

	for (int i = 0; i < 1;++i) {
	createPointLight("PointLight" + std::to_string(i), 500.0f , glm::vec3{0.0f, 10.0f, 0.0f});
	}
}

void Engine::sRendering()
{
}


void Engine::sInput()
{

	//camera.processMouseInput(static_cast<float>(xMouseOffset), static_cast<float>(yMouseOffset), static_cast<float>(scrollOffset));
	//camera.processKeyboardInput(m_window, static_cast<float>(m_deltaTime));

	//xMouseOffset = 0.0f;
	//yMouseOffset = 0.0f;
	//scrollOffset = 0.0f;
}

void Engine::deltaTimeUpdate()
{
	auto currentTime = static_cast<float>(glfwGetTime());
	m_deltaTime = currentTime - m_lastFrameTime;
	m_lastFrameTime = currentTime;
}




Handle<Model> Engine::loadModel(const std::string& file) const
{
	const auto fullPath = FileManager::getPath(file);
	if (!FileManager::fileExists(fullPath))
	{
		std::cerr << "ENGINE:: Cannot load this model.\n";
		return Handle<Model>::InvalidHandle;
	}

	return m_modelLoader->loadModel(file);
}


void Engine::createTextures()

{
	m_assetManager->textures.addNamed(Texture("assets/Textures/empty.jpg"), "default");
	m_assetManager->textures.addNamed(Texture("assets/Textures/aphex.gif"), "aphex");
	auto floorTex = m_assetManager->textures.addNamed(Texture("assets/Textures/bricks2.jpg", Texture::Diffuse, true, GL_REPEAT), "floor");
	m_assetManager->textures.addNamed(Texture("assets/Textures/bricks2_normal.jpg", Texture::Normal, true, GL_REPEAT), "floor_normal");
	m_assetManager->textures.addNamed(Texture("assets/Textures/bricks2_disp.jpg", Texture::Height, true, GL_REPEAT), "floor_disp");
	m_assetManager->textures.addNamed(Texture("assets/Textures/container_diffuse.png"), "container");
}


void Engine::createShaders()
{
	m_assetManager->shaders.addNamed(Shader("assets/Shaders/default.vert", "assets/Shaders/default_lit.frag"), "lit");
	m_assetManager->shaders.addNamed(Shader("assets/Shaders/default.vert", "assets/Shaders/default_unlit.frag"), "unlit");
	m_assetManager->shaders.addNamed(Shader("assets/Shaders/default.vert", "assets/Shaders/deferred/deferred.frag"), SHADER_DEFAULT_TEXTURED_LIT);
	m_assetManager->shaders.addNamed(Shader("assets/Shaders/world_grid.vert", "assets/Shaders/world_grid.frag"), "grid");
	m_assetManager->shaders.addNamed(Shader("assets/Shaders/item_icon.vert", "assets/Shaders/item_icon.frag"), "icon");
	m_assetManager->shaders.addNamed(Shader("assets/Shaders/default.vert", "assets/Shaders/singleColor.frag"), "border");
	m_assetManager->shaders.addNamed(Shader("assets/Shaders/screen.vert", "assets/Shaders/screen.frag"), "screenShader");
	m_assetManager->shaders.addNamed(Shader("assets/Shaders/shadow/shadowMap.vert", "assets/Shaders/shadow/shadowMap.frag"), "shadowMap");

	m_assetManager->shaders.addNamed(Shader("assets/Shaders/shadow/pointMap.vert", "assets/Shaders/shadow/pointMap.frag", "assets/Shaders/shadow/pointMap.geom"), "pointMap");
	m_assetManager->shaders.addNamed(Shader("assets/Shaders/screen.vert", "assets/Shaders/hdr.frag"), "HDR");
	m_assetManager->shaders.addNamed(Shader("assets/Shaders/screen.vert", "assets/Shaders/bloom_blur.frag"), "bloomBlur");
	m_assetManager->shaders.addNamed(Shader("assets/Shaders/screen.vert", "assets/Shaders/bloom_new.frag"), "bloom");

	m_assetManager->shaders.addNamed(Shader("assets/Shaders/default.vert", "assets/Shaders/deferred/deferred.frag"), "deferredShader");
	m_assetManager->shaders.addNamed(Shader("assets/Shaders/screen.vert", "assets/Shaders/deferred/lightPass.frag"), "deferredLightPass");

	m_assetManager->shaders.addNamed(Shader("assets/Shaders/screen.vert", "assets/Shaders/ssao/ssao.frag"), "ssaoShader");
	m_assetManager->shaders.addNamed(Shader("assets/Shaders/screen.vert", "assets/Shaders/ssao/ssaoBlur.frag"), "ssaoBlur");


}

void Engine::createModels()
{
	Handle<Material> container = m_assetManager->materials.getNamedHandle("container");
	auto* cont = m_assetManager->materials.get(container);
	cont->setColor(1.0f, 1.0f, 1.0f);
	Handle<Material> def = m_assetManager->materials.getNamedHandle("default");
	Handle<Material> floor = m_assetManager->materials.getNamedHandle("floor");
	m_assetManager->models.addNamed(Model(ModelSet{std::move(Mesh(Cube::vertices, Cube::indices)), container}), "cube");
	m_assetManager->models.addNamed(Model(ModelSet{ std::move(Mesh(Plane::vertices, Plane::indices)), def}), "plane");
	m_assetManager->models.addNamed(Model(ModelSet{ std::move(Mesh(Plane::vertices, Plane::indices)), floor }), "floor");
}

void Engine::createMaterials()
{
	m_assetManager->materials.addNamed(Material(m_assetManager->shaders.getNamedHandle("lit")), "default");

	auto litHandle = m_assetManager->materials.addNamed(Material(m_assetManager->shaders.getNamedHandle(SHADER_DEFAULT_TEXTURED_LIT)), "lit");

	auto* mat = m_assetManager->materials.get(litHandle);
	mat->setColor(999.0f, 1.0f, 333.0f);
	m_assetManager->materials.addNamed(Material(m_assetManager->shaders.getNamedHandle("unlit")), "unlit");

	m_assetManager->materials.addNamed(Material(m_assetManager->shaders.getNamedHandle(SHADER_DEFAULT_TEXTURED_LIT),
		m_assetManager->textures.getNamedHandle("aphex")), "aphex");

	Handle<Material> floorMat = m_assetManager->materials.addNamed(Material(m_assetManager->shaders.getNamedHandle(SHADER_DEFAULT_TEXTURED_LIT), m_assetManager->textures.getNamedHandle("floor")), "floor");
	m_assetManager->materials.get(floorMat)->setShininess(16.0f);
	m_assetManager->materials.get(floorMat)->setSpecular(0.15f);
	m_assetManager->materials.get(floorMat)->addTexture(m_assetManager->textures.getNamedHandle("floor_normal"));
	m_assetManager->materials.get(floorMat)->addTexture(m_assetManager->textures.getNamedHandle("floor_disp"));

	Handle<Texture> containerTex = m_assetManager->textures.getNamedHandle("container");

	auto containerHandle = m_assetManager->materials.addNamed(Material(m_assetManager->shaders.getNamedHandle(SHADER_DEFAULT_TEXTURED_LIT), containerTex) , "container");

	m_assetManager->materials.get(containerHandle)->addTexture(m_assetManager->textures.addNamed(Texture("assets/Textures/container_specular.png", Texture::Type::Specular), "container_specular"));

}

void Engine::createObjectIcons()
{
	Handle<Texture> directionalLightIcon = m_assetManager->textures.addNamed(Texture("assets/Textures/light-icon.png"), "icon_directionalLight");
	IconRegistry::registerType<LightComponent>(m_assetManager->textures.get(directionalLightIcon));

}



Entity* Engine::createFloor()
{
	auto* floor = m_currentScene->createEntity("Floor");
	floor->addComponent<MeshComponent>(m_assetManager->models.getNamed("floor"));
	floor->transform->m_scale = glm::vec3(50.0f);

	return (floor);
}

void Engine::addMeshToScene(Model* model, glm::vec3 position)
{
	auto* entity = m_currentScene->createEntity("NewObject");
	entity->addComponent<MeshComponent>(model);
	entity->transform->m_position = (position);
}




void Engine::createPointLight(const std::string& name, float radius, glm::vec3 position)
{
	auto* light = m_currentScene->createEntity(name);
	auto* lightComp = light->addComponent<LightComponent>(LightComponent::Type::Point);
	light->transform->m_position = (position);
	std::get<PointLightSettings>(lightComp->m_settings).m_attenuationRadius = radius;
}

Entity* Engine::createDirectionalLight(const std::string& name, glm::vec3 direction)
{
	auto* light = m_currentScene->createEntity(name);
	auto* lightComp = light->addComponent<LightComponent>(LightComponent::Type::Directional);
	std::get<DirectionalLightSettings>(lightComp->m_settings).m_direction = direction;
	std::get<DirectionalLightSettings>(lightComp->m_settings).m_intensity = 1.0f;

	return light;
}

void Engine::createCube(const std::string& name, const char* materialName, glm::vec3 position, float rotationAngle, glm::vec3 rotationAxis,
                        glm::vec3 scale)
{
	auto cube = m_currentScene->createEntity("Cube");
	cube->addComponent<MeshComponent>(m_assetManager->models.getNamed("cube"));

	cube->transform->m_position = position;
	cube->transform->m_rotationAxis = rotationAxis;
	cube->transform->m_rotationAngle = rotationAngle;
	cube->transform->m_scale = scale;
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


