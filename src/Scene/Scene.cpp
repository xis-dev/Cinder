#include "Scene.h"
#include "Resources/Shader.h"



void Scene::imguiUse(const std::unique_ptr<Entity>& entity)
{
	entity->imguiDraw();
}

void Scene::applyLightCountsToShader(const Shader& shader)
{
	shader.setUniformi("u_DirLightCount", DirectionalLight::m_lightCountByType);
	shader.setUniformi("u_PointLightCount", PointLight::m_lightCountByType);
	shader.setUniformi("u_SpotLightCount", SpotLight::m_lightCountByType);

}


void Scene::illuminate(const Shader& shader)
{
	for (auto& light : m_lights)
	{
		light->use(shader);
	}
}


