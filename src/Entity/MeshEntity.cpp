#include "Entity/MeshEntity.h"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "glm/ext/matrix_transform.hpp"

#include "imgui.h"

#include "Material.h"
#include "Mesh.h"
#include "Resources/ResourceManager.h"


MeshEntity::MeshEntity(Mesh* mesh)
{
	m_meshes.push_back(mesh);
}




void MeshEntity::imguiDraw()
{
	Entity::imguiDraw();

	ImGui::DragFloat3("Scale", &m_scale.x);
}

glm::mat4 MeshEntity::getTransformMatrix()
{
	glm::mat4 modelMatrix{ 1.0f };
	modelMatrix = glm::translate(modelMatrix, static_cast<glm::vec3>(getPosition()));
	modelMatrix = glm::rotate(modelMatrix, glm::radians((getRotationAngle())), static_cast<glm::vec3>(getRotationAxis()));
	modelMatrix = glm::scale(modelMatrix, static_cast<glm::vec3>(getScale()));
	return modelMatrix;
}

void MeshEntity::render(const Shader& shader)
{
	Entity::render(shader);
	for (auto& mesh : m_meshes)
	{
		mesh->draw(shader);
	}
}
