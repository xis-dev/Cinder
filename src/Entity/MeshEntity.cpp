#include "Entity/MeshEntity.h"


#include "Resources/ResourceManager.h"
#include "Resources/Model.h"

#include "glm/ext/matrix_transform.hpp"

#include "imgui.h"







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

