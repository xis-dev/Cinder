#include "Scene.h"
#include "Resources/Shader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/matrix.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "imgui.h"

#include <algorithm>

Entity * Scene::getRoot() const
{
	return RootEntity;
}

void Scene::init(AssetManager* asset_manager)
{
	ASSET_MANAGER = asset_manager;

	auto empty = std::make_unique<Entity>();
	RootEntity = empty.get(); // Root entity should still point to empty
	RootEntity->setTag("SceneRoot");
	m_entities.push_back(std::move(empty));
}


void Scene::destroyEntity(Entity *entity)
{
	if (entity->isPendingDestruction())
	{
		std::cerr << "Trying to destroy: " << entity->getTag() << " but is already marked for destruction.\n";
		return;
	}
	entity->m_pendingDestruction = true;
	m_entitiesPendingDestruction.push_back(entity);
}

void Scene::end()
{
	for (auto* ent: m_entitiesPendingDestruction)
	{
		if (!ent)
		{
			std::cout << "Scene attempting to destroy null entity.\n";
			continue;
		}
		OnEntityDestroyed.broadcast(ent);
		ent->OnDestroyed();
		ent->removeParent();
		ent->reparentAllChildren(*RootEntity); // Reset all direct children back to root

		// Find and remove entity's unique pointer
		auto entUniqueIt = std::find_if(m_entities.begin(), m_entities.end(), [ent](const auto& unique){return unique.get() == ent;});
		if (entUniqueIt != m_entities.end())
		{
			m_entities.erase(entUniqueIt);
		}

	}

	m_entitiesPendingDestruction.clear();
}


