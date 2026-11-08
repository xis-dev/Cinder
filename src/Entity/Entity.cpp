#include "Entity/Entity.h"

#include "Resources/Shader.h"

#include "imgui.h"

#include <algorithm>

Entity::Entity()
{
	transform = addComponent<Transform>();
}

const std::unordered_map<ComponentTypeID, std::unique_ptr<Component>> & Entity::getComponents() const
{
	return m_components;
}

void Entity::findAndRemoveChild(Entity *child)
{
	if (auto childIterator = std::find_if(m_children.begin(), m_children.end(), [child](const Entity* entPtr){return entPtr == child;});
			 childIterator != m_children.end())
	{
		m_children.erase(childIterator);
	}
}

void Entity::setParent(Entity& parent)
{
	if (m_parent)
	{
		m_parent->findAndRemoveChild(this);
	}
	m_parent = &parent;
	parent.m_children.push_back(this);
}

Entity * Entity::getParent() const
{
	return m_parent;
}

void Entity::addChild(Entity& child)
{
	child.setParent(*this);
}

void Entity::removeParent()
{
	if (m_parent)
	{
		m_parent->findAndRemoveChild(this);
		m_parent = nullptr;
	}
}

void Entity::reparentAllChildren(Entity& newParent)
{
	auto childrenCopy = m_children;
	for (auto* childEnt: childrenCopy)
	{
		if (childEnt)
		{
			childEnt->setParent(newParent);
		}
	}
	m_children.clear();
}


bool Entity::isPendingDestruction()
{
	return m_pendingDestruction;
}

std::vector<Entity *> Entity::getChildren() const
{
	return m_children;
}



void Entity::setTag(const std::string& tag)
{
	m_tag = tag;
}

std::string Entity::getTag() const
{
	return m_tag;

}

void Entity::imguiDraw()
{
	ImGui::InputText("Entity Name", m_tag.data(), 32);
	ImGui::DragFloat3("Position", &transform->m_position.x, 0.5f);
	ImGui::DragFloat3("Rotation Axis", &transform->m_rotationAxis.x, 0.1f);
	ImGui::DragFloat("Angle", &transform->m_rotationAngle);
}

void Entity::OnDestroyed()
{
	// TODO: Keep scale, position and rotation of children after destruction
}

