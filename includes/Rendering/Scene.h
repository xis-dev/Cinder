#pragma once
#include <vector>

#include <memory>
#include <concepts>
#include <ranges>
#include <type_traits>
#include <unordered_map>
#include <Utilities/IconRegistry.h>
#include "Entity/Entity.h"
#include "Entity/LightEntity.h"
#include "Entity/MeshEntity.h"
#include "Resources/Model.h"
#include "Resources/Mesh.h"
#include "Resources/Material.h"

class Shader;
class Camera;

class Scene
{

public:
	Scene() = default;

private:

	size_t m_totalEntities{};
	std::vector<std::unique_ptr<Entity>> m_entities{};
	std::vector<LightEntity*> m_lights{};
public:
	std::vector<MeshEntity*> m_meshEnts{};
	//std::unordered_map<Shader*, std::vector<MeshEntity*>> m_renderBatches{};

public:
	template<typename T, typename... TArgs>
		requires std::derived_from<T, Entity>
	T* createEntity(const std::string& name, TArgs&&... args)
	{
		auto entity = std::make_unique<T>(std::forward<TArgs>(args)...);
		entity->setTag(name);
		m_entities.push_back(std::move(entity));

		auto* basePtr = m_entities.back().get();
		T* typePtr = dynamic_cast<T*>(basePtr);
		if constexpr (std::is_base_of_v<LightEntity, T>)
		{
			m_lights.push_back(dynamic_cast<LightEntity*>(typePtr));
			typePtr->setLightID(T::m_lightCountByType);
			++T::m_lightCountByType;
		}

		if constexpr (std::is_base_of_v<MeshEntity, T>)
		{
			m_meshEnts.push_back(typePtr);
		}
		//	Model* model = typePtr->getModel();
		//	for (const auto &mat: model->getMeshes() | std::views::keys)  // or getModelMap()
		//	{
		//		if (mat != nullptr)
		//		{
		//			if (Shader* shader = mat->tryGetShader())
		//			{
		//				m_renderBatches[shader].push_back(typePtr);
		//			}
		//		}
		//	}
		//}

		if (auto icon = IconRegistry::tryGetIcon<T>())
		{
			typePtr->setIcon(*icon);
		}

		return typePtr;
	}

		
	

public:
	
	void applyLightCountsToShader(const Shader& shader) const;


	const std::vector<std::unique_ptr<Entity>>& getEntities() const
	{
		return m_entities;
	}

	size_t getEntityCount() const
	{
		return m_entities.size();
	}

	//const std::unordered_map<Shader*, std::vector<MeshEntity*>>& getRenderBatches() const
	//{
	//	return m_renderBatches;
	//}

	void illuminate(const Shader& shader) const;

	void render(const Camera& cam) const;


};

 