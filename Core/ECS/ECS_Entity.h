#pragma once
#include <bitset>
#include <ranges>

#include <vector>
#include <tuple>
#include <unordered_map>
#include <Core/Headers/Components.h>

#include "Entity.h"
#include "Material.h"
#include "map"
#include "Mesh.h"
#include "Objects/Headers/Camera.h"

class Scene;
class Entity;
class EntityMemoryPool;


namespace ECSConstants
{
	// Number of individual components
	constexpr size_t MAX_COMPONENTS = 16;

}

template <typename T>
concept ComponentConcept = std::derived_from<T, Component>;
class EntityMemoryPool {

	friend Entity;
	friend Scene;

	static size_t m_maxEntities;

	size_t currentComponentID{};
	std::vector<std::string> m_entityTags{};
	std::vector<bool> m_entityActive{};
	std::map<size_t, std::vector<Transform>> m_TransformMap{};
	std::map<size_t, std::vector<MeshRenderer>> m_RendererMap{};
	std::tuple<std::vector<Transform>,
		std::vector<MeshRenderer>,
		std::vector<Light>,
		std::vector<PointLight>,
		std::vector<SpotLight>,
		std::vector<DirectionalLight>> m_pool;

	std::unordered_map<std::string, size_t> components{};

	static void init(size_t maxEntities)
	{
		m_maxEntities = maxEntities;
		Instance();
	}

	EntityMemoryPool()
	{
		m_entityActive.reserve(m_maxEntities);
		m_entityTags.reserve(m_maxEntities);

		std::vector<Transform> cTransforms{};
		cTransforms.reserve(m_maxEntities);
		registerComponent<Transform>();

		std::vector<MeshRenderer> cMeshRenderers{};
		cMeshRenderers.reserve(m_maxEntities);
		registerComponent<MeshRenderer>();


		std::vector<Light> cLights{};
		cLights.reserve(m_maxEntities);
		registerComponent<Light>();

		std::vector<DirectionalLight> cDirectionalLights{};
		cDirectionalLights.reserve(m_maxEntities);
		registerComponent<DirectionalLight>();

		std::vector<SpotLight> cSpotLights{};
		cSpotLights.reserve(m_maxEntities);
		registerComponent<SpotLight>();

		std::vector<PointLight> cPointLights{};
		cPointLights.reserve(m_maxEntities);
		registerComponent<PointLight>();


		for (size_t i = 0; i < m_maxEntities; i++)
		{
			m_entityActive.emplace_back();
			m_entityTags.emplace_back();

			cTransforms.emplace_back();
			cMeshRenderers.emplace_back();
			cLights.emplace_back();
			cDirectionalLights.emplace_back();
			cSpotLights.emplace_back();
			cPointLights.emplace_back();
		}

		m_pool = std::tuple{ cTransforms, cMeshRenderers, cLights, cPointLights, cSpotLights, cDirectionalLights };
	}

	template <ComponentConcept T>
	void registerComponent()
	{
		auto typeName = typeid(T).name();
		// TODO: loop over components
		if (!components.contains(typeName))
		{
			components.insert({ typeName, currentComponentID });
			currentComponentID++;
		}
	}

	void addEntity(const size_t id, const std::string& tag)
	{
		m_entityTags[id] = tag;
		m_entityActive[id] = true;
	}

	void removeEntity(size_t id)
	{
		m_entityActive[id] = false;
	}

	size_t getAvailableId()
	{
		size_t nextIndex{};
		for (size_t i = 0; i < m_maxEntities; i++)
		{
			if (m_entityActive[i]) continue;
			nextIndex = i;
			break;

		}
		return nextIndex;
	}


public:

	static EntityMemoryPool& Instance()
	{
		static EntityMemoryPool instance{};
		return instance;
	}

	template <typename T, typename... TArgs>
	T& addComponent(int id, TArgs... args)
	{
		auto& component = std::get<std::vector<T>>(m_pool)[id];
		component = T(std::forward<TArgs>(args)...);
		component.active = true;

		return component;
	}

	template <typename T>
	T& getComponent(int id)
	{
		return std::get<std::vector<T>>(m_pool)[id];
	}


};






class Camera;

class Entity {
	friend Scene;

	Entity() = default;
	Entity(size_t id) : m_id(id)
	{

	}

	size_t m_id{};
	std::bitset<ECSConstants::MAX_COMPONENTS> componentBitset{};

	[[nodiscard]] int getID() const;
public:
	[[nodiscard]] const std::string& getTag() const;

	// TODO: Add error checking, component does not exist
	template <typename T, typename... TArgs>
	T& addComponent(TArgs&&... args)
	{
		return EntityMemoryPool::Instance().addComponent<T>(m_id, std::forward<TArgs>(args)...);
	}


	template <typename T>
	T& getComponent() const
	{
		auto& component = EntityMemoryPool::Instance().getComponent<T>(m_id);
		return component;
	}



	template <typename T>
	bool hasComponent() const
	{
		return getComponent<T>().active;
	}


	void destroy()
	{
		EntityMemoryPool::Instance().removeEntity(m_id);
	}

	bool operator== (const Entity rhs) const
	{
		return m_id == rhs.m_id;
	}

	bool operator!=(const Entity rhs) const
	{
		return !(operator==(rhs));
	}





};

