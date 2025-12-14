#pragma once
#include <vector>
#include "Core/Headers/Entity.h"
#include "Core/Headers/LightEntity.h"
#include "Core/Headers/MeshEntity.h"
#include "Math/Vec3.h"
#include <Resources/Headers/Shader.h>
#include <memory>
#include <concepts>
#include <type_traits>
#include <Core/IconRegistry.h>

class Shader;

class Scene
{

	struct RenderBatch {
		Shader* shader;
		std::vector<MeshEntity*> meshes;
	};
public:
	Scene() = default;

private:

	size_t m_totalEntities{};
	std::vector<std::unique_ptr<Entity>> m_entities{};
	std::vector<LightEntity*> m_lights{};
	std::unordered_map<Shader*, std::vector<MeshEntity*>> m_renderBatches{};

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
			auto& meshes = typePtr->getMeshes();
			for (auto* mesh : meshes)
			{
				for (auto* mat : mesh->getMaterials())
				{
					m_renderBatches[mat->tryGetShader()].push_back(typePtr);
				}
			}
		}

		if (auto icon = IconRegistry::tryGetIcon<T>())
		{
			typePtr->setIcon(*icon);
		}

		return typePtr;
	}

		
	

public:
	
	void imguiUse(const std::unique_ptr<Entity>& entity);
	void applyLightCountsToShader(const Shader& shader);


	const std::vector<std::unique_ptr<Entity>>& getEntities() const
	{
		return m_entities;
	}

	size_t getEntityCount() const
	{
		return m_entities.size();
	}

	const std::unordered_map<Shader*, std::vector<MeshEntity*>>& getRenderBatches()
	{
		return m_renderBatches;
	}

	void illuminate(const Shader& shader);


};

 