#pragma once
#include <bitset>
#include <ranges>

#include <vector>
#include <tuple>
#include <unordered_map>
#include <Core/Headers/Components.h>

#include "Material.h"
#include "map"
#include "Mesh.h"
#include "Objects/Headers/Camera.h"

// New entity includes
#include "imgui.h"

class Scene;
class Old_Entity;
class EntityMemoryPool;


namespace ECSConstants {
	// Number of individual components
	constexpr size_t MAX_COMPONENTS = 16 ;

}

class Entity 
{
public:
	virtual ~Entity() = default;

protected:
	Vec3f m_position{};
	Vec3f m_currentRotationAxis{Vec3f(0.0f, 1.0f, 0.0f)};
	float m_currentRotationAngle{};

public:

	Vec3f getPosition() { return m_position;}
	Vec3f getRotationAxis() { return m_currentRotationAxis; }
	float getRotationAngle() { return m_currentRotationAngle; }

	Vec3f setPosition(Vec3f pos) { m_position = pos; }
	Vec3f setPosition(float p) { m_position = Vec3f(p); }


	void setRotation(Vec3f axis, float angle);

	virtual void imguiDraw();
	// TODO: In spirit of generalization, add gizmos(?)
	virtual void render() = 0;
};

class MeshEntity : public Entity {
public:
	MeshEntity() = default;
	MeshEntity(const std::shared_ptr<Mesh> mesh, const std::shared_ptr<Material> material);

private:
	std::vector<std::shared_ptr<Mesh>> m_meshes{};
	std::vector<std::shared_ptr<Material>> m_materials{};

	Vec3f m_scale{1.0f};

public:
	Vec3f getScale() { return m_scale; }
	Vec3f setScale(Vec3f scale) { m_scale = scale; }
	Vec3f setScale(float s) { m_scale = Vec3f(s); }

	virtual void imguiDraw() override;
	virtual void render() override;
};

template <typename T>
concept ComponentConcept = std::derived_from<T, Component>;
class EntityMemoryPool{

	friend Old_Entity;
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

		m_pool = std::tuple{cTransforms, cMeshRenderers, cLights, cPointLights, cSpotLights, cDirectionalLights};
	}

	template <ComponentConcept T>
	void registerComponent()
	{
		auto typeName = typeid(T).name();
		// TODO: loop over components
		if (!components.contains(typeName))
		{
			components.insert({typeName, currentComponentID});
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

class Old_Entity
{
	friend Scene;

	Old_Entity() = default;
	Old_Entity(size_t id): m_id(id)
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

	bool operator== (const Old_Entity rhs) const
	{
		return m_id == rhs.m_id;
	}

	bool operator!=(const Old_Entity rhs) const
	{
		return !(operator==(rhs));
	}





};

