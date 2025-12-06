#include "Scene.h"

void Scene::imguiUse(const std::unique_ptr<Entity>& entity)
{
	entity->imguiDraw();
}

void Scene::applyLightCountsToShader(const std::shared_ptr<Shader>& shader)
{
	shader->setUniformi("u_DirLightCount", DirectionalLight::m_lightCountByType);
	shader->setUniformi("u_PointLightCount", PointLight::m_lightCountByType);
	shader->setUniformi("u_SpotLightCount", SpotLight::m_lightCountByType);

}


void Scene::illuminate(const std::shared_ptr<Shader>& shader)
{
	shader->use();
	for (auto& light : m_lights)
	{
		light->use(shader);
	}
}


