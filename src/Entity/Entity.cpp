#include "Entity/Entity.h"

#include "Resources/Shader.h"

#include "imgui.h"

void Entity::setRotation(Vec3f axis, float angle)
{
	m_currentRotationAxis = axis;
	m_currentRotationAngle = angle;
}

void Entity::setTag(const std::string& tag)
{
	if (tag.length() > MAX_NAME_LENGTH)
	{
		std::cout << "ENTITY:: Attempting to name entity with more than " << MAX_NAME_LENGTH << " characters. Consider reducing name length.\n";
		std::string shortenedTag{ tag };
		shortenedTag.erase(MAX_NAME_LENGTH, std::string::npos);
		strcpy(m_tag, shortenedTag.c_str());
		return;
	}
	strcpy(m_tag, tag.c_str());
}

const char* Entity::getTag()
{
	return m_tag;
}

void Entity::setIcon(Texture& icon)
{
	m_hasIcon = true;
	m_icon = &icon;
}

bool Entity::hasIcon()
{
	return m_hasIcon;
}

Texture* Entity::tryGetIcon()
{
	if (!m_icon)
	{
		std::cerr << "ENTITY:: Current entity named: " << m_tag << " has no icon.\n";
		return nullptr;
	}
	return m_icon;
}

const Texture* Entity::tryGetIcon() const
{
	if (!m_icon)
	{
		std::cerr << "ENTITY:: Current entity named: " << m_tag << " has no icon.\n";
		return nullptr;
	}
	return m_icon;
}

void Entity::imguiDraw()
{
	ImGui::InputText("Entity Name", m_tag, MAX_NAME_LENGTH);
	ImGui::DragFloat3("Position", &m_position.x, 0.5f);
	ImGui::DragFloat3("Rotation Axis", &m_currentRotationAxis.x, 0.1f);
	ImGui::DragFloat("Angle", &m_currentRotationAngle);
}

glm::mat4 Entity::getTransformMatrix()
{
	glm::mat4 modelMatrix{ 1.0f };
	modelMatrix = glm::translate(modelMatrix, static_cast<glm::vec3>(getPosition()));
	modelMatrix = glm::rotate(modelMatrix, glm::radians((getRotationAngle())), static_cast<glm::vec3>(getRotationAxis()));
	return modelMatrix;
}

void Entity::render(const Shader& shader)
{
	shader.setUniformMat4("u_ModelMatrix", getTransformMatrix());
}
