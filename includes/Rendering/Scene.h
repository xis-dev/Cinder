#pragma once
#include <vector>

#include <memory>
#include <concepts>
#include <ranges>
#include <type_traits>
#include <unordered_map>
#include <Utilities/IconRegistry.h>

#include "AssetManager.h"
#include "Components/Transform.h"
#include "Entity/Entity.h"
#include "Entity/LightEntity.h"
#include "Entity/MeshEntity.h"
#include "ext/matrix_clip_space.hpp"
#include "Resources/Model.h"
#include "Resources/Mesh.h"
#include "Resources/Material.h"

#include "glm/mat4x4.hpp"

class Shader;
class Camera;

struct RenderBatch
{
	Shader* shader;
	ModelSet* modelSet;
};
struct PointShadow
{
	unsigned shadowCubemap{};
	std::vector<glm::mat4> shadowMapTransforms{};
};
class Scene
{

public:
	Scene() = default;
private:

	size_t m_totalEntities{};
	AssetManager* ASSET_MANAGER{nullptr};

public:
	Entity* RootEntity{nullptr};
	std::vector<std::unique_ptr<Entity>> m_entities{};
	// TODO: Add pending destruction state for entities with handles
	std::vector<Entity*> m_entitiesPendingDestruction;
	// TODO: Replace when doing proper batching with instancing and materials
//	std::unordered_map<Entity*, std::unordered_map<Shader*,std::vector<const ModelSet*>>> m_modelSetsToRemove;
public:
//	std::unordered_map<Shader*, std::unordered_map<const ModelSet*, Entity*>> m_renderBatches{};

public:
	Entity* createEntity(const std::string& name)
	{
		auto entity = std::make_unique<Entity>();
		Entity* rawPtr = entity.get();
		rawPtr->setTag(name);
		RootEntity->addChild(*rawPtr);

		m_entities.push_back(std::move(entity));

		return rawPtr;
	}



public:

	Delegate<Entity*> OnEntityDestroyed;
	Entity* getRoot() const;
	void init(AssetManager* assetManager);

	template <CinUtility::DerivedConcept<Component>... TComponents>
	[[nodiscard]] std::vector<Entity*> getEntitiesByComponents() const;

	template <CinUtility::DerivedConcept<Component> T>
	std::vector<T*> getComponentFromEntities() const;
	void imguiRender();

	// TODO: Move to renderer
	void setupPointMatrices(int w, int h);

	[[nodiscard]] const std::vector<std::unique_ptr<Entity>>& getEntities() const
	{
		return m_entities;
	}

	size_t getEntityCount() const
	{
		return m_entities.size();
	}

	void illuminate(const Shader& shader) const;

	void render(const Camera& cam) const;

	void destroyEntity(Entity* entity);

	void end();

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

 