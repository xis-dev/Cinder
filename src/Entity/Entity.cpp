#include "Entity/Entity.h"

#include "Resources/Shader.h"

#include "imgui.h"

void Entity::findAndRemoveChild(Entity *child)
{
	for (size_t i = 0; i < m_children.size(); ++i)
	{
		if (m_children[i] == child)
		{
		m_children.erase(m_children.begin() + i);
		}
	}
}

void Entity::setParent(Entity* parent)
{
	if (m_parent)
	{
		m_parent->findAndRemoveChild(this);
	}
	m_parent = parent;
	parent->m_children.push_back(this);
}

std::vector<Entity *> Entity::getChildren() const
{
	return m_children;
}

glm::vec3 Entity::getWorldPosition() const
{
	if (m_parent)
	{
		glm::mat4 posMat{1.0};
		posMat = glm::translate(posMat, m_parent->getWorldPosition());
		return glm::vec3(posMat * glm::vec4(getRelativePosition(), 1.0));
	}
	return getRelativePosition();
}


void Entity::setRotation(
	glm::vec3 axis, float angle)
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

const char * Entity::getTag() const
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

glm::mat4 Entity::getRelativeTransformMatrix()
{
	glm::mat4 modelMatrix{ 1.0f };
	modelMatrix = glm::translate(modelMatrix, static_cast<glm::vec3>(getRelativePosition()));
	modelMatrix = glm::rotate(modelMatrix, glm::radians((getRelativeRotationAngle())), static_cast<glm::vec3>(getRelativeRotationAxis()));
	return modelMatrix;
}

glm::mat4 Entity::getGlobalTransformMatrix()
{
	glm::mat4 transform{1.0};
	Entity* current = this;
	while (current)
	{
		transform = current->getRelativeTransformMatrix() * transform;
		current = current->m_parent;
	}
	return transform;
}

