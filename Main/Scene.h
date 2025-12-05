#pragma once
#include <vector>
#include <Core/Headers/Components.h>
#include "Core/Headers/Entity.h"
#include "Math/Vec3.h"
#include <Objects/Headers/Shader.h>
class Scene
{
public:
	Scene(size_t maxEntitiesInScene);
	

private:

	size_t m_totalEntities{};
	std::vector<Old_Entity> m_entities{};
	std::vector<Old_Entity> dirLightsInScene{};
	std::vector<Old_Entity> pointLightsInScene{};
	std::vector<Old_Entity> spotLightsInScene{};

	int directionalLightCount{};
	int pointLightCount{};
	int spotLightCount{};

public:
	Old_Entity addDirLight(const std::string& name, Vec3f direction = Vec3f(0.0f, -1.0f, 0.0f), Vec3f color = Vec3f(1.0f), float intensity = 1.0f);
	Old_Entity addPointLight(const std::string& name, float radius, Vec3f position = Vec3f(0.0f), Vec3f color = Vec3f(1.0f), float intensity = 1.0f);
	Old_Entity addSpotLight(const std::string& name, Vec3f position = Vec3f(0.0f), Vec3f direction = Vec3f(0.0f, -1.0f, 0.0f), float innerAngle = 0.0f, float outerAngle = 0.0f, Vec3f color = Vec3f(1.0f), float intensity = 1.0f );

	void applyLightCountsToShader(Shader &shader);

	const std::vector<Old_Entity>& getEntities() const
	{
		return m_entities;
	}

	const std::vector<Old_Entity>& getDirLights() const
	{
		return dirLightsInScene;
	}

	const std::vector<Old_Entity>& getPointLights() const
	{
		return pointLightsInScene;
	}

	const std::vector<Old_Entity>& getSpotLights() const
	{
		return spotLightsInScene;
	}
	size_t getEntityCount() const
	{
		return m_totalEntities;
	}

	int getDirectionalLightCount() const
	{
		return dirLightsInScene.size();
	}
	int getPointLightCount() const
	{
		return pointLightsInScene.size();
	}

	int getSpotLightCount() const
	{
		return spotLightsInScene.size();
	}
	Old_Entity addEntity(const std::string& name);

};
