#pragma once
#include <vector>

#include <memory>
#include <Utilities/IconRegistry.h>

#include "AssetManager.h"
#include "Camera.h"
#include "Components/Transform.h"
#include "Entity/Entity.h"



class Scene
{

public:
	Scene() = default;
private:

	std::vector<std::unique_ptr<Entity>> m_entities{};
	AssetManager* ASSET_MANAGER{nullptr};
	std::unique_ptr<Camera> m_camera{};

public:
	Entity* RootEntity{nullptr};
	// TODO: Add pending destruction state for entities with handles
	std::vector<Entity*> m_entitiesPendingDestruction;

public:
	Entity* createEntity(const std::string& name);

public:

	Delegate<Entity*> OnEntityDestroyed;
	[[nodiscard]] Entity* getRoot() const;
	void init(AssetManager *asset_manager, const Camera &cam);

	template <CinUtility::DerivedConcept<Component>... TComponents>
	[[nodiscard]] std::vector<Entity*> getEntitiesByComponents() const;

	template <CinUtility::DerivedConcept<Component> T>
	std::vector<T*> getComponentFromEntities() const;

	[[nodiscard]] const std::vector<std::unique_ptr<Entity>>& getEntities() const{return m_entities;}

	[[nodiscard]] size_t getEntityCount() const{return m_entities.size();}

	void destroyEntity(Entity* entity);

	void end();

	Camera* getViewCamera() const;

};


template<CinUtility::DerivedConcept<Component> ... TComponents>
std::vector<Entity *> Scene::getEntitiesByComponents() const
{
	std::vector<Entity*> out{};

	for (const auto& ent: m_entities)
	{
		if ((ent->hasComponent<TComponents>() && ...))
		{
			out.push_back(ent.get());
		}
	}

	return out;
}

template<CinUtility::DerivedConcept<Component> T>
std::vector<T *> Scene::getComponentFromEntities() const
{
	std::vector<T*> out{};
	for (const auto& ent: m_entities)
	{
		if (T* component = ent->getComponent<T>()) out.push_back(component);
	}

	return out;
}

 