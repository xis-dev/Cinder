#include "Scene.h"

Scene::Scene(size_t maxEntitiesInScene)
{
	EntityMemoryPool::init(maxEntitiesInScene);
}

Old_Entity Scene::addDirLight(const std::string& name, Vec3f direction, Vec3f color, float intensity)
{
	Old_Entity dirLight = addEntity(name);
	dirLight.addComponent<DirectionalLight>(direction, dirLightsInScene.size(), color, intensity);
	dirLightsInScene.push_back(dirLight);
	return dirLight;
}

Old_Entity Scene::addPointLight(const std::string& name, float radius, Vec3f position, Vec3f color, float intensity)
{
	Old_Entity pointLight = addEntity(name);
	pointLight.addComponent<Transform>(position);
	pointLight.addComponent<PointLight>(radius, pointLightsInScene.size(), color, intensity);
	pointLightsInScene.push_back(pointLight);
	return pointLight;
}

Old_Entity Scene::addSpotLight(const std::string& name, Vec3f position, Vec3f direction , float innerAngle, float outerAngle, Vec3f color, float intensity)
{
	Old_Entity spotLight = addEntity(name);
	spotLight.addComponent<Transform>(position);
	spotLight.addComponent<SpotLight>(direction, innerAngle, outerAngle, spotLightsInScene.size(), color, intensity);
	spotLightsInScene.push_back(spotLight);
	return spotLight;
}

void Scene::applyLightCountsToShader(Shader& shader)
{
	shader.setUniformi("u_DirLightCount", static_cast<int>(dirLightsInScene.size()));
	shader.setUniformi("u_PointLightCount", static_cast<int>(pointLightsInScene.size()));
	shader.setUniformi("u_SpotLightCount", static_cast<int>(spotLightsInScene.size()));

}

Old_Entity Scene::addEntity(const std::string& name)
{
	auto& pool = EntityMemoryPool::Instance();
	size_t id = pool.getAvailableId();
	pool.addEntity(id, name);
	
	auto ent = Old_Entity(id);
	m_entities.push_back(ent);
	m_totalEntities++;
	return ent;
}
