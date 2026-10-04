#pragma once


#include <format>
#include <memory>

#include "Math/Vec3.h"
#include "Delegate.h"
#include "AABB.h"

#include "glm/ext/matrix_transform.hpp"

#include <string>
#include <vector>

#include "CinUtility.h"
#include "Component.h"

class Scene;

class Shader;
class Texture;

typedef uint32_t ComponentTypeID;

namespace ComponentUtil
{
	inline ComponentTypeID nextComponentID{};

	template<CinUtility::DerivedConcept<Component> T>
	ComponentTypeID getComponentID()
	{
		static ComponentTypeID id = nextComponentID++;
		return id;
	}
}

class Entity 
{
	friend Scene;
	static constexpr size_t MAX_NAME_LENGTH = 32;


public:
	virtual ~Entity() = default;

private:
	std::string m_tag;
	Entity* m_parent {nullptr};
	std::vector<Entity*> m_children;
	std::unordered_map<ComponentTypeID, std::unique_ptr<Component>> m_components;

public:
	template<CinUtility::DerivedConcept<Component> T, typename ... TArgs>
	T* addComponent(TArgs&&... args);

	template<CinUtility::DerivedConcept<Component> T>
	T* getComponent();

	template<CinUtility::DerivedConcept<Component> T>
	const T* getComponent() const;

	template<CinUtility::DerivedConcept<Component> T>
	[[nodiscard]] bool hasComponent() const;

protected:
	//AABB m_boundingBox;

	glm::vec3 m_position{};
	glm::vec3 m_currentRotationAxis{glm::vec3(0.0f, 1.0f, 0.0f)};
	float m_currentRotationAngle{};

	Texture* m_icon{};
	bool m_hasIcon{};

	bool m_pendingDestruction{};

	// Returns success state boolean
	void findAndRemoveChild(Entity* child);
public:

	void setParent(Entity* parent);
	Entity* getParent() const;
	void addChild(Entity* child);
	void removeParent();
	void reparentAllChildren(Entity* newParent);

	bool isPendingDestruction();

	[[nodiscard]] std::vector<Entity*> getChildren() const;
	[[nodiscard]] glm::vec3 getRelativePosition() const { return m_position;}
	[[nodiscard]] glm::vec3 getWorldPosition() const;
	[[nodiscard]] glm::vec3 getRelativeRotationAxis() const { return m_currentRotationAxis; }
	float getRelativeRotationAngle() const{ return m_currentRotationAngle; }

	void setPosition(glm::vec3 pos) { m_position = pos; }
	void setPosition(float p) { m_position = glm::vec3(p); }


	void setRotation(glm::vec3 axis, float angle);

	void setTag(const std::string& tag);

	const char* getTag() const;

	virtual void setIcon(Texture& icon);

	bool hasIcon();

	Texture* tryGetIcon();

	const Texture* tryGetIcon() const;

	virtual void imguiDraw();

	virtual glm::mat4 getRelativeTransformMatrix();

	// Transform matrix globally after parent transformation
	virtual glm::mat4 getGlobalTransformMatrix();

	Delegate<Entity*> OnEntityDestroyed_WithEntity;
	Delegate<> OnEntityDestroyed;

protected:
	virtual void OnDestroyed();
};

template<CinUtility::DerivedConcept<Component> T, typename ... TArgs>
T * Entity::addComponent(TArgs &&...args){

	std::unique_ptr<Component> newComponent = std::make_unique<T>(std::forward<TArgs>(args)...);
	T* rawPtr = newComponent.get();

	try {
		if (!m_components.insert({ComponentUtil::getComponentID<T>(), std::move(newComponent)}))
		{
			std::cout << std::format("ENTITY_LOG: Attempt to add component {ID: %i} to entity '%s' with already exising component, request will be ignored.\n", ComponentUtil::getComponentID<T>(), m_tag);

			// Return actual existing component
			return m_components[ComponentUtil::getComponentID<T>()];
		}
	}
	catch (const std::bad_alloc& badAlloc) {
		std::cout << std::format("Caught exception while inserting component {ID %i} to entity '%s': ", ComponentUtil::getComponentID<T>(), m_tag) << badAlloc.what() << "\n";

		return nullptr;
	}

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


