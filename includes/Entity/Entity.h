#pragma once

#include "Delegate.h"
#include "AABB.h"

#include "glm/ext/matrix_transform.hpp"

#include <string>
#include <vector>
#include <format>
#include <memory>
#include <iostream>
#include <unordered_map>

#include "CinUtility.h"
#include "Component.h"
#include "Components/Transform.h"

class Scene;

class Shader;
class Texture;

// TODO: Change all Entity refs back to pointers, make constructor private.
class Entity
{
	// TODO: Should be used to allow private entity constructor but then cant use entity refs
	friend Scene;

public:
	Entity();
	virtual ~Entity() = default;

private:
	std::string m_tag{};
	Entity* m_parent {nullptr};
	std::vector<Entity*> m_children;
	std::unordered_map<ComponentTypeID, std::unique_ptr<Component>> m_components;

public:
	Transform* transform{nullptr};

	template<CinUtility::DerivedConcept<Component> T, typename ... TArgs>
	requires std::constructible_from<T, TArgs...>
	T* addComponent(TArgs&&... args);

	template<CinUtility::DerivedConcept<Component> T>
	T* getComponent();

	template<CinUtility::DerivedConcept<Component> T>
	const T* getComponent() const;

	template<CinUtility::DerivedConcept<Component> T>
	[[nodiscard]] bool hasComponent() const;

	const std::unordered_map<ComponentTypeID, std::unique_ptr<Component>> & getComponents() const;
protected:
	//AABB m_boundingBox;

	bool m_pendingDestruction{};

public:

	void setParent(Entity& parent);
	Entity* getParent() const;
	void addChild(Entity& child);
	void removeParent();
	void reparentAllChildren(Entity& newParent);
	void findAndRemoveChild(Entity* child);
	bool isPendingDestruction();

	std::vector<Entity *> getChildren() const;

	void setTag(const std::string& tag);

	std::string getTag() const;

	virtual void imguiDraw();

	Delegate<Entity*> OnEntityDestroyed_WithEntity;
	Delegate<> OnEntityDestroyed;

protected:
	virtual void OnDestroyed();
};

template<CinUtility::DerivedConcept<Component> T, typename ... TArgs>
requires std::constructible_from<T, TArgs...>
T * Entity::addComponent(TArgs &&...args){

	const auto& it = m_components.find(ComponentUtil::getComponentID<T>());

	if (it != m_components.end())
	{
		std::cout << std::format("ENTITY_LOG: Attempt to add component 'ID: {}' to entity '{}' with already exising component, request will be ignored.\n", ComponentUtil::getComponentID<T>(), m_tag);
		return nullptr;
	}

	std::unique_ptr<T> newComponent = std::make_unique<T>(std::forward<TArgs>(args)...);
	T* rawPtr = newComponent.get();

	m_components.insert({ComponentUtil::getComponentID<T>(), std::move(newComponent)});

	return rawPtr;

}

template<CinUtility::DerivedConcept<Component> T>
T * Entity::getComponent()
{
	const auto& it = m_components.find(ComponentUtil::getComponentID<T>());
	if (it != m_components.end()) return static_cast<T*>(it->second.get());

	return nullptr;
}

template<CinUtility::DerivedConcept<Component> T>
const T * Entity::getComponent() const
{
	const auto& it = m_components.find(ComponentUtil::getComponentID<T>());
	if (it != m_components.end()) return static_cast<const T*>(it->second.get());

	return nullptr;
}

template<CinUtility::DerivedConcept<Component> T>
bool Entity::hasComponent() const
{
	return m_components.contains(ComponentUtil::getComponentID<T>());
}


