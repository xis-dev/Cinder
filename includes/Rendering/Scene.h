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
#include "ext/matrix_clip_space.hpp"
#include "Resources/Model.h"
#include "Resources/Mesh.h"
#include "Resources/Material.h"

#include "glm/mat4x4.hpp"

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
	std::vector<glm::mat4> dirLightTransforms{};
	//std::unordered_map<Shader*, std::vector<MeshEntity*>> m_renderBatches{};

public:
	template<typename T, typename... TArgs>
		requires std::derived_from<T, Entity>
	T* createEntity(const std::string& name, TArgs&&... args)
	{
		auto entity = std::make_unique<T>(std::forward<TArgs>(args)...);
		T* rawPtr = entity.get();
		rawPtr->setTag(name);
		m_entities.push_back(std::move(entity));

		if constexpr (std::is_base_of_v<LightEntity, T>)
		{
			if constexpr (std::is_same_v<T, DirectionalLight>)
			{
				const float dist = 50.0f;
				auto dir = rawPtr->m_direction.getNormalized();
				glm::mat4 proj = glm::ortho(-35.0f, 35.0f, -35.0f, 35.0f, 0.1f, 75.0f);
				glm::mat4 view = glm::lookAt(static_cast<glm::vec3>(-dir) * 20.0f, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				dirLightTransforms.push_back(proj * view);
			}
			m_lights.push_back(dynamic_cast<LightEntity*>(rawPtr));
			rawPtr->setLightID(T::m_lightCountByType);
			++T::m_lightCountByType;
		}

		if constexpr (std::is_base_of_v<MeshEntity, T>)
		{
			m_meshEnts.push_back(rawPtr);
		}


		if (auto icon = IconRegistry::tryGetIcon<T>())
		{
			rawPtr->setIcon(*icon);
		}

		return rawPtr;
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

	void illuminate(const Shader& shader) const;

	void render(const Camera& cam) const;


};

 