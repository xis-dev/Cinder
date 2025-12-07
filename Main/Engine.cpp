#include "Main/Engine.h"
#include <ranges>
#include <Shader.h>
#include <Resources/ResourceManager.h>


GLFWwindow* Engine::m_window{};

bool Engine::cullBackface{};

bool Engine::drawWireframe{};

Vec3f cubePositions[] = {
	Vec3f( 0.0f,  0.0f,  0.0f),
	Vec3f( 2.0f,  5.0f, -15.0f),
	Vec3f(-1.5f, -2.2f, -2.5f),
	Vec3f(-3.8f, -2.0f, -12.3f),
	Vec3f( 2.4f, -0.4f, -3.5f),
	Vec3f(-1.7f,  3.0f, -7.5f),
	Vec3f( 1.3f, -2.0f, -2.5f),
	Vec3f( 1.5f,  2.0f, -2.5f),
	Vec3f( 1.5f,  0.2f, -1.5f),
	Vec3f(-1.3f,  1.0f, -1.5f)
};
glm::vec3 pointLightPositions[] = {
	glm::vec3( 0.7f,  0.2f,  2.0f),
	glm::vec3( 2.3f, -3.3f, -4.0f),
	glm::vec3(-4.0f,  2.0f, -12.0f),
	glm::vec3( 0.0f,  0.0f, -3.0f)
};
void Engine::run()
{
	// initialize the engine, main loop
	// TODO: replace and construct better with systems
	init(m_window);
	while (!glfwWindowShouldClose(m_window))
	{
		glfwPollEvents();

		deltaTimeUpdate();

		imguiUse();
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



void Engine::init(GLFWwindow*& window)
{
	// initialize glfw and give window hints for the version and the opengl profile we want to use
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create the glfw window
	window = glfwCreateWindow(screen::width, screen::height, screen::title, nullptr, nullptr);

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

	glfwSwapInterval(0);
	// set callbacks, input mode and enable depth
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glfwSetFramebufferSizeCallback(m_window, Engine::frameBufferSizeCallback);
	glfwSetScrollCallback         (m_window, Engine::scrollCallback);
	glfwSetCursorPosCallback      (m_window, Engine::mouseCallback);
	glfwSetKeyCallback            (m_window, Engine::keyCallback);
	glfwSetInputMode              (m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	imguiInit();


	// as is obvious, materials must be created after shaders and textures
	createTextures();
	createShaders();
	createMeshes();
	createMaterials();

	createObjectIcons();

	initializeAddonVAO();


	//loadModel("C:/Users/PC/Downloads/backpack/backpack.obj", "Bag", "lit", Vec3f(0.0f, 1.0f, 7.0f));
	//loadModel("c:/users/pc/desktop/c++/glscene/models/Chest_LowPoly.obj", "Chest", "default", Vec3f(0.0f, 10.0f, 0.0f), Vec3f(5.0f));

	for (int i = 1; i < 10; ++i)
	{
		createCube("cube", "container",
			cubePositions[i], 20.0f * i,
			cubePositions[i].getNormalized(),
			Vec3f(2.0f));
	}

	for (int i = 0; i < 4;++i) {
	createPointLight("PointLight" + std::to_string(i), 500.0f ,Vec3f(pointLightPositions[i].x, pointLightPositions[i].y, pointLightPositions[i].z));

	}


	//createFloor();
	createDirectionalLight("DirectionalLight", Vec3f(0.0f, -10.0f, 5.0f));
	createDirectionalLight("DirectionalLight", Vec3f(0.0f, -5.0f, 5.0f));


	// TODO: change scene creation



}

void Engine::sRendering()
{
	// Tell opengl what color we want glClear to clear the color buffer
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	 // Clear color and depth buffers


	renderGrid();
	if (drawWireframe)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	glm::vec3 cameraPosition = camera.getPosition();
	glm::mat4 projectionMat = camera.getProjectionMatrix();
	glm::mat4 viewMat = camera.getViewMatrix();
	glm::mat4 vpMat = projectionMat * viewMat;

		for (auto& [name, shader] : ResourceManager<Shader>::getAllResources())
		{
			m_currentScene.illuminate(shader);
			m_currentScene.applyLightCountsToShader(shader);

			shader->setUniformVec3("u_CameraPosition", cameraPosition);
			shader->setUniformMat4("u_ProjectionMatrix", projectionMat);
			shader->setUniformMat4("u_ViewMatrix", viewMat);
			shader->setUniformMat4("u_VPMatrix", vpMat);

			shader->setUniformi("u_cullBackface", cullBackface);
			shader->setUniformVec3("u_ViewDirection", camera.getDirection());

			for (auto& entity : m_currentScene.getEntities())
			{

				shader->setUniformMat4("u_MVPMatrix", vpMat * entity->getTransformMatrix());
				entity->render(shader);
			}
		}

	
	glClear(GL_DEPTH_BUFFER_BIT);
	renderIcons();


}


void Engine::sInput()
{

	camera.processMouseInput(m_window, static_cast<float>(screen::xOffset), static_cast<float>(screen::yOffset), static_cast<float>(screen::scrollOffset));
	camera.processKeyboardInput(m_window, static_cast<float>(m_deltaTime));

	screen::xOffset = 0.0f;
	screen::yOffset = 0.0f;
	screen::scrollOffset = 0.0f;
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

void Engine::imguiUse()
{
	// setup imgui for a new frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// initialize and assign entity names for imgui dropdown
	std::vector<const char*> entityNames{};
	entityNames.reserve(m_currentScene.getEntities().size());
	for (auto& e : m_currentScene.getEntities())
	{
		entityNames.push_back(e->getTag());
	}

	std::vector<const char*> materialNames{};
	
		for (const auto& key : ResourceManager<Material>::getAllResources() | std::views::keys)
		{
			materialNames.push_back(key.c_str());
		}

	

	ImGui::Begin("Linking ts broke my ass");

	if (ImGui::BeginTabBar("Content"))
	{
		if (ImGui::BeginTabItem("Entities"))
		{
			static int entityIndex{};
			ImGui::Combo("Entities", &entityIndex, entityNames.data(), static_cast<int>(entityNames.size()));
			if (entityIndex >= 0 && !m_currentScene.getEntities().empty())
			{
				auto& currentEntity = m_currentScene.getEntities()[entityIndex];
				m_currentScene.imguiUse(currentEntity);
			}


			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Materials"))
		{
			static int materialIndex{};
			ImGui::Combo("Materials", &materialIndex, materialNames.data(), static_cast<int>(materialNames.size()));
			if (materialIndex >= 0)
			{
				std::shared_ptr<Material> currentMaterial = ResourceManager<Material>::getResource(materialNames[materialIndex]);
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

				currentMaterial->setColor(Vec3f(materialColor[0], materialColor[1], materialColor[2]));
				currentMaterial->setDiffuse(diffuseStr);
				currentMaterial->setAmbience(ambientStr);
				currentMaterial->setSpecular(specularStr);
				currentMaterial->setShininess(shininess);
			}
			ImGui::EndTabItem();
		}


		ImGui::EndTabBar();

	}

	ImGui::Checkbox("Enable Backface Culling", &cullBackface);
	ImGui::Checkbox("Draw Wireframe", &drawWireframe);
	std::string deltaTimeText{};
	std::string frameRateText{};
	deltaTimeText = "Delta Time: " + std::to_string(static_cast<int>(std::floor(1000 / (1 / m_deltaTime)))) + " ms/s";
	frameRateText = "FPS: " + std::to_string(static_cast<int>(std::floor(1 / m_deltaTime)));
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

void Engine::renderGrid()
{
	auto gridShader = ResourceManager<Shader>::getResource("grid");
	gridShader->use();
	gridShader->setUniformMat4("u_VPMatrix", (camera.getProjectionMatrix() * camera.getViewMatrix()));
	gridShader->setUniformVec3("u_CameraPosition", camera.getPosition());
	
	drawAddon(6);
}

void Engine::renderIcons()
{
	glm::mat4 view = camera.getViewMatrix();
	glm::mat4 projection = camera.getProjectionMatrix();

	auto iconShader = ResourceManager<Shader>::getResource("icon");
	iconShader->use();
	iconShader->setUniformMat4("u_ViewMatrix", view);
	iconShader->setUniformMat4("u_ProjectionMatrix", projection);

	glm::vec3 cameraRightWorldSpace = glm::vec3{ view[0][0], view[1][0], view[2][0] };
	glm::vec3 cameraUpWorldSpace = glm::vec3{ view[0][1], view[1][1], view[2][1] };
	iconShader->setUniformVec3("u_CameraRight_WorldSpace", cameraRightWorldSpace);

	iconShader->setUniformVec3("u_CameraUp_WorldSpace", cameraUpWorldSpace);

	for (auto& entity : m_currentScene.getEntities())
	{
		if (entity->hasIcon())
		{
			iconShader->use();

			iconShader->setUniformVec3("u_ObjectPosition", (glm::vec3)entity->getPosition());

			glActiveTexture(GL_TEXTURE0);
			entity->getIcon()->use();
			iconShader->setUniformi("u_iconImage", 0);
			drawAddon(6);
		}
	}
}

void Engine::drawAddon(int indexCount)
{
	glBindVertexArray(addonVAO);
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)indexCount);
	glBindVertexArray(0);
}


void Engine::createTextures()
{
	ResourceManager<Texture>::addResource("default", std::make_shared<Texture>("Textures/empty.jpg"));
	ResourceManager<Texture>::addResource("aphex", std::make_shared<Texture>("Textures/aphex.gif"));
	ResourceManager<Texture>::addResource("floor", std::make_shared<Texture>("Textures/woodenFloor.jpg"));
	ResourceManager<Texture>::addResource("container", std::make_shared<Texture>("Textures/container_diffuse.png"));
	ResourceManager<Texture>::addResource("container_specular", std::make_shared<Texture>("Textures/container_specular.png", Texture::Specular));
}


void Engine::createShaders()
{
	ResourceManager<Shader>::addResource("lit", Shader::getDefaultShader());
	ResourceManager<Shader>::addResource("unlit", std::make_shared<Shader>("Shaders/default.vert", "Shaders/default_unlit.frag"));
	ResourceManager<Shader>::addResource("textured_lit", Shader::getTexturedShader());
	ResourceManager<Shader>::addResource("grid", std::make_shared<Shader>("Shaders/world_grid.vert", "Shaders/world_grid.frag"));
	ResourceManager<Shader>::addResource("icon", std::make_shared<Shader>("Shaders/item_icon.vert", "Shaders/item_icon.frag"));
}

void Engine::createMeshes()
{
	ResourceManager<Mesh>::addResource("cube", std::make_shared<Mesh>(Cube::vertices, Cube::indices));
	ResourceManager<Mesh>::getResource("cube")->tag = "cube";
	ResourceManager<Mesh>::addResource("plane", std::make_shared<Mesh>(Floor::vertices, Floor::indices));
	ResourceManager<Mesh>::getResource("plane")->tag = "plane";

}

void Engine::createMaterials()
{
	ResourceManager<Material>::addResource("default", std::make_shared<Material>(ResourceManager<Shader>::getResource("lit")));

	ResourceManager<Material>::addResource("lit", std::make_shared<Material>(ResourceManager<Shader>::getResource("textured_lit")));

	ResourceManager<Material>::addResource("unlit", std::make_shared<Material>(ResourceManager<Shader>::getResource("unlit")));

	ResourceManager<Material>::addResource("aphex", std::make_shared<Material>(ResourceManager<Shader>::getResource("textured_lit"), 
		ResourceManager<Texture>::getResource("aphex")));

	ResourceManager<Material>::addResource("floor", std::make_shared<Material>(ResourceManager<Shader>::getResource("textured_lit"), ResourceManager<Texture>::getResource("floor")));

	ResourceManager<Material>::addResource("container", std::make_shared<Material>(ResourceManager<Shader>::getResource("textured_lit"), ResourceManager<Texture>::getResource("container")));

	ResourceManager<Material>::getResource("container")->addTexture(ResourceManager<Texture>::getResource("container_specular"));
}

void Engine::createObjectIcons()
{

	IconRegistry::registerType<DirectionalLight>("Textures/light-icon.png");
	IconRegistry::registerType<PointLight>("Textures/light-icon.png");
	IconRegistry::registerType<SpotLight>("Textures/light-icon.png");

}

void Engine::initializeAddonVAO()
{
	glGenVertexArrays(1, &addonVAO);
	glBindVertexArray(addonVAO);
	glBindVertexArray(0);

}

void Engine::createIconVAO()
{
	
}

void Engine::createFloor()
{
}


void Engine::sendGeneralShaderUniforms()
{
	
}

void Engine::createPointLight(const std::string& name, float radius, Vec3f position)
{
	auto* light = m_currentScene.createEntity<PointLight>(name, radius);
	light->setPosition(position);
}

void Engine::createDirectionalLight(const std::string& name, Vec3f direction)
{
	auto* light = m_currentScene.createEntity<DirectionalLight>(name, direction);
}

void Engine::createCube(const std::string& name, const char* materialName, Vec3f position, float rotationAngle, Vec3f rotationAxis,
                        Vec3f scale)
{
	auto* cube = m_currentScene.createEntity<MeshEntity>("Cube", ResourceManager<Mesh>::getResource("cube"), ResourceManager<Material>::getResource(materialName));
	cube->setPosition(position);
	cube->setRotation(rotationAxis, rotationAngle);
	cube->setScale(scale);
}

void Engine::loadModel(const char* path, const char* tag, const char* materialName, Vec3f position, Vec3f scale, float angle, Vec3f rotAxis)
{

}



void Engine::keyCallback(GLFWwindow* window, int key, int action, int scancode, int mods)
{

	if (glfwGetKey(m_window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
	{
		screen::shiftLock = !screen::shiftLock;
		if (screen::shiftLock)
		{
			screen::firstMouse = true;
			glfwSetCursorPos(m_window, static_cast<float>(screen::width) / 2.0f, static_cast<float>(screen::height) / 2.0f);
		}
		glfwSetInputMode(m_window, GLFW_CURSOR, screen::shiftLock ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
	}
}

void Engine::frameBufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);

	screen::width = width;
	screen::height = height;
}

void Engine::mouseCallback(GLFWwindow* window, double xPos, double yPos)
{
	if (!screen::shiftLock) return;
	if (screen::firstMouse)
	{
		screen::lastX = xPos;
		screen::lastY = yPos;
		screen::firstMouse = false;
	}

	screen::xOffset = xPos - screen::lastX;
	screen::yOffset = screen::lastY - yPos;

	screen::lastX = xPos;
	screen::lastY = yPos;
}

void Engine::scrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
	screen::scrollOffset = yOffset;
}



