#include "Main/Engine.h"
#include <ranges>
#include <Shader.h>
#include <Resources/ResourceManager.h>
#include <Core/IconRegistry.h>

#include "Core/Headers/LightEntity.h"
#include "Core/Headers/MeshEntity.h"
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
	createMaterials();
	createMeshes();

	createObjectIcons();

	initializeAddonVAO();


	loadModel("C:/Users/PC/Downloads/backpack/backpack.obj", materials.tryGetResource("lit"));
	//loadModel("c:/users/pc/desktop/c++/glscene/models/Chest_LowPoly.obj", "Chest", "default", Vec3f(0.0f, 10.0f, 0.0f), Vec3f(5.0f));

	/*for (int i = 1; i < 2; ++i)
	{
		createCube("cube", "container",
			cubePositions[i], 20.0f * i,
			cubePositions[i].getNormalized(),
			Vec3f(2.0f));
	}*/

	createDirectionalLight("DirectionalLight", Vec3f(0.0f, -10.0f, 5.0f));


	for (int i = 0; i < 4;++i) {
	createPointLight("PointLight" + std::to_string(i), 500.0f ,Vec3f(pointLightPositions[i].x, pointLightPositions[i].y, pointLightPositions[i].z));

	}


	//createFloor();


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


		for (auto [shader, instances] : m_currentScene.getRenderBatches())
		{
			shader->use();
			m_currentScene.illuminate(*shader);
			m_currentScene.applyLightCountsToShader(*shader);

			shader->setUniformVec3("u_CameraPosition", cameraPosition);
			shader->setUniformMat4("u_ProjectionMatrix", projectionMat);
			shader->setUniformMat4("u_ViewMatrix", viewMat);
			shader->setUniformMat4("u_VPMatrix", vpMat);

			shader->setUniformi("u_cullBackface", cullBackface);
			shader->setUniformVec3("u_ViewDirection", camera.getDirection());

			for (auto* inst : instances)
			{
				shader->setUniformMat4("u_MVPMatrix", vpMat * inst->getTransformMatrix());
				inst->render(*shader);
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

	auto matNames = materials.getAllResourceNames();
	std::vector<const char*> matNamesPtr{};
	matNamesPtr.reserve(matNames.size());

	for (auto& s : matNames)
	{
		matNamesPtr.push_back(s.c_str());
	}

	auto meshNames = meshes.getAllResourceNames();
	std::vector<const char*> meshNamesPtr{};
	meshNamesPtr.reserve(meshNames.size());

	for (auto& s : meshNames)
	{
		meshNamesPtr.push_back(s.c_str());
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
			ImGui::Combo("Materials", &materialIndex, matNamesPtr.data(), static_cast<int>(matNamesPtr.size()));
			if (materialIndex >= 0)
			{
				if (Material* currentMaterial = materials.tryGetResource(matNamesPtr[materialIndex]))
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

					currentMaterial->setColor(Vec3f(materialColor[0], materialColor[1], materialColor[2]));
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
				if (Mesh* currentMesh = meshes.tryGetResource(meshNamesPtr[meshIndex]))
				{
					static float posToAdd[3]{};
					ImGui::DragFloat3("Position to add Mesh", posToAdd);
					if (ImGui::Button("Add Mesh", ImVec2(100, 50)))
					{
						addMeshToScene(currentMesh, Vec3f(posToAdd[0], posToAdd[1], posToAdd[2]));
					}

				}
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
	if (auto gridShader = shaders.tryGetResource("grid"))
	{
		gridShader->use();
		gridShader->setUniformMat4("u_VPMatrix", (camera.getProjectionMatrix() * camera.getViewMatrix()));
		gridShader->setUniformVec3("u_CameraPosition", camera.getPosition());

		drawAddon(6);
	}
	
}

void Engine::renderIcons()
{

	if (auto iconShader = shaders.tryGetResource("icon"))
	{
		glm::mat4 view = camera.getViewMatrix();
		iconShader->use();
		glm::mat4 projection = camera.getProjectionMatrix();

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
				iconShader->setUniformMat4("u_ProjectionMatrix", projection);
				iconShader->setUniformMat4("u_ViewMatrix", view);

				glActiveTexture(GL_TEXTURE0);
				entity->tryGetIcon()->use();
				iconShader->setUniformi("u_iconImage", 0);
				drawAddon(6);
			}
		}
	}
	
	
}

void Engine::drawAddon(int indexCount)
{
	glBindVertexArray(addonVAO);
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)indexCount);
	glBindVertexArray(0);
}

void Engine::loadModel(const std::string& filePath, Material* mat)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ENGINE::ASSIMP:: " << importer.GetErrorString() << std::endl;
		return;
	}

	std::string directory = filePath.substr(0, filePath.find_last_of('/'));

	processNode(scene->mRootNode, scene, directory, mat);
}

void Engine::processNode(aiNode* node, const aiScene* scene, const std::string& directory, Material* mat)
{
	for (unsigned i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.addResource(mesh->mName.C_Str(), processMesh(mesh, scene, directory, mat));
	}

	for (unsigned i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene, directory, mat);
	}
}

Mesh Engine::processMesh(aiMesh* mesh, const aiScene* scene, const std::string& directory, Material* mat)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned> indices;
	std::vector<Texture*> ts;
	for (unsigned i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		glm::vec3 vector;
		
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		
		vertex.position = Vec3f(vector.x, vector.y, vector.z);

		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;

		vertex.normal = Vec3f(vector.x, vector.y, vector.z);

		if (mesh->mTextureCoords[0])
		{
			glm::vec2 vec;
			
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;

			vertex.texCoords = Vec2f(vec.x, vec.y);
		}
		else
		{
			vertex.texCoords = Vec2f(0.0f, 0.0f);
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

	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture*> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, Texture::Diffuse, directory);
		
		ts.insert(ts.end(), diffuseMaps.begin(), diffuseMaps.end());
		std::vector<Texture*> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, Texture::Specular, directory);

		ts.insert(ts.end(), specularMaps.begin(), specularMaps.end());

		for (auto* tex : ts)
		{
			mat->addTexture(tex);
		}

	}

	return Mesh(vertices, indices, mat);
}

std::vector<Texture*> Engine::loadMaterialTextures(aiMaterial* mat, aiTextureType assimp_textureType, Texture::Type textureType, const std::string& directory)
{
	std::vector<Texture*> ts;

	for (unsigned i = 0; i < mat->GetTextureCount(assimp_textureType); i++)
	{
		aiString str;
		mat->GetTexture(assimp_textureType, i, &str);
		std::string texturePath = directory + '/' + str.C_Str();
		bool skip = false;

		auto allTextures = textures.getAllResources();
		for (unsigned j = 0; j < allTextures.size(); j++)
		{
			if (std::strcmp(allTextures[j].second->getLocation().c_str(), FileManager::getAbsolutePath(texturePath).c_str()) == 0)
			{
				ts.push_back(textures.tryGetResource(allTextures[j].first));
				std::cout << "same tex.\n";
				skip = true;
				break;
			}
		}
		if (!skip)
		{
			std::string textureName = texturePath.substr(texturePath.find_last_of('/'), texturePath.find_last_of('.'));
			auto* tex = textures.addOrReplaceResource(textureName, Texture(texturePath, textureType));
			ts.push_back(tex);
		}	
	}
	return ts;
}


void Engine::createTextures()
{
	textures.addResource("default", Texture("Textures/empty.jpg"));
	textures.addResource("aphex", Texture("Textures/aphex.gif"));
	textures.addResource("floor", Texture("Textures/woodenFloor.jpg"));
	textures.addResource("container", Texture("Textures/container_diffuse.png"));
	textures.addResource("container_specular", Texture("Textures/container_specular.png", Texture::Specular));
}


void Engine::createShaders()
{
	shaders.addResource("lit", Shader("Shaders/default.vert", "Shaders/default_lit.frag"));
	shaders.addResource("unlit", Shader("Shaders/default.vert", "Shaders/default_unlit.frag"));
	shaders.addResource("textured_lit", Shader("Shaders/default.vert", "Shaders/textured_lit.frag"));
	shaders.addResource("grid", Shader("Shaders/world_grid.vert", "Shaders/world_grid.frag"));
	shaders.addResource("icon", Shader("Shaders/item_icon.vert", "Shaders/item_icon.frag"));
}

void Engine::createMeshes()
{
	meshes.addResource("cube", Mesh(Cube::vertices, Cube::indices, materials.tryGetResource("container")));
	meshes.addResource("plane", Mesh(Floor::vertices, Floor::indices, materials.tryGetResource("default")));

}

void Engine::createMaterials()
{
	materials.addResource("default", Material(shaders.tryGetResource("lit")));

	materials.addResource("lit", Material(shaders.tryGetResource("textured_lit")));

	materials.addResource("unlit", Material(shaders.tryGetResource("unlit")));

	materials.addResource("aphex", Material(shaders.tryGetResource("textured_lit"), 
		textures.tryGetResource("aphex")));

	materials.addResource("floor", Material(shaders.tryGetResource("textured_lit"), textures.tryGetResource("floor")));

	materials.addResource("container", Material(shaders.tryGetResource("textured_lit"), textures.tryGetResource("container")));


	materials.tryGetResource("container")->addTexture(textures.tryGetResource("container_specular"));
}

void Engine::createObjectIcons()
{
	Texture* directionalLightIcon = textures.addResource("icon_directionalLight", Texture("Textures/light-icon.png"));
	Texture* pointLightIcon = textures.addResource("icon_pointLight", Texture("Textures/light-icon.png"));
	Texture* spotLightIcon = textures.addResource("icon_spotLight", Texture("Textures/light-icon.png"));

	IconRegistry::registerType<DirectionalLight>(*directionalLightIcon);
	IconRegistry::registerType<PointLight>(*pointLightIcon);
	IconRegistry::registerType<SpotLight>(*spotLightIcon);

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

void Engine::addMeshToScene(Mesh* mesh, Vec3f position)
{
	auto* entity = m_currentScene.createEntity<MeshEntity>("NewObject", mesh);
	entity->setPosition(position);
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
	light->setPosition(3.0f);
}

void Engine::createCube(const std::string& name, const char* materialName, Vec3f position, float rotationAngle, Vec3f rotationAxis,
                        Vec3f scale)
{
	auto cube = m_currentScene.createEntity<MeshEntity>("Cube", meshes.tryGetResource("cube"));
	cube->setPosition(position);
	cube->setRotation(rotationAxis, rotationAngle);
	cube->setScale(scale);
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



