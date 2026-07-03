#include "Scene.h"
#include "Resources/Shader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/matrix.hpp>
#include <glm/ext/matrix_clip_space.hpp>

#include "imgui.h"


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

void Scene::applyLightCountsToShader(const Shader& shader) const
{
	shader.setUniformi("u_DirLightCount", DirectionalLight::m_lightCountByType);
	shader.setUniformi("u_PointLightCount", PointLight::m_lightCountByType);
	shader.setUniformi("u_SpotLightCount", SpotLight::m_lightCountByType);

}

void Scene::imguiRender()
{
	//imguiRenderEntityHierarchy(RootEntity);
}

void Scene::setupPointMatrices(int w, const int h)
{
	for (auto& ps: m_pointShadows)
	{
		const auto* light = ps.first;
		auto&[shadowCubemap, shadowMapTransforms] = ps.second;
		shadowMapTransforms.clear();
		float aspect = (float)w / (float)h;
		float near = 1.0f;
		glm::mat4 shadow_proj = glm::perspective(glm::radians(90.0f), aspect, near, light->m_radius);

		glm::vec3 lightPos = light->getRelativePosition();
		shadowMapTransforms.push_back(shadow_proj *
			glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		shadowMapTransforms.push_back(shadow_proj *
			glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		shadowMapTransforms.push_back(shadow_proj *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
		shadowMapTransforms.push_back(shadow_proj *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
		shadowMapTransforms.push_back(shadow_proj *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
		shadowMapTransforms.push_back(shadow_proj *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));
	}


}



void Scene::illuminate(const Shader& shader) const
{
	for (auto& light : m_lights)
	{
		light->use(shader);
	}
}

void Scene::render(const Camera &cam) const
{

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
	if (auto* meshEnt = dynamic_cast<MeshEntity*>(entity)) // Mesh entity to remove from render batches
	{
		std::vector<const ModelSet*> modelSetList;
		for (const ModelSet& modelSet: meshEnt->getModel()->getMeshes())
		{
			Shader* shader = ASSET_MANAGER->shaders.get(ASSET_MANAGER->materials.get(modelSet.mat)->getShader());
			m_modelSetsToRemove[entity][shader].push_back(&modelSet);
		}
	}
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
		ent->reparentAllChildren(RootEntity); // Reset all direct children back to root
		// Remove from render batch
		for (auto& [shader, modelSets] : m_modelSetsToRemove[ent]) // get shader and model sets based on map to entity for removal
		{
			// Map of model set and entity from current shader;
			auto& modelSetAndEntity = m_renderBatches[shader];
			for (const ModelSet* set: modelSets)
			{
				auto modelSetAndEntityIterator = modelSetAndEntity.find(set);
				if (modelSetAndEntityIterator == modelSetAndEntity.end()) return;
				modelSetAndEntity.erase(modelSetAndEntityIterator);
			}
		}

		// Remove from mesh ents or light containers
		if (auto* meshEnt = dynamic_cast<MeshEntity*>(ent))
		{
			auto meshEntIt = std::find_if(m_meshEnts.begin(), m_meshEnts.end(), [meshEnt](const MeshEntity* m){return m == meshEnt;});
			if (meshEntIt != m_meshEnts.end())
			{
				m_meshEnts.erase(meshEntIt);
			}
		}
		else if (auto* lightEnt = dynamic_cast<LightEntity*>(ent))
		{
			auto lightEntIt = std::find_if(m_lights.begin(), m_lights.end(), [lightEnt](const LightEntity* l){return l == lightEnt;});
			if (lightEntIt != m_lights.end())
			{
				m_lights.erase(lightEntIt);
			}

			if (auto* pointLight = dynamic_cast<PointLight*>(lightEnt))
			{

				auto pointLightIt = m_pointShadows.find(pointLight);
				if (pointLightIt != m_pointShadows.end())
				{
					m_pointShadows.erase(pointLightIt);
				}
			}
		}


		// Find and remove entity's unique pointer
		auto entUniqueIt = std::find_if(m_entities.begin(), m_entities.end(), [ent](const auto& unique){return unique.get() == ent;});
		if (entUniqueIt != m_entities.end())
		{
			m_entities.erase(entUniqueIt);
		}

	}

	m_entitiesPendingDestruction.clear();
	m_modelSetsToRemove.clear();
}


