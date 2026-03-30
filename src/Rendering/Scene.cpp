#include "Scene.h"
#include "Resources/Shader.h"



void Scene::applyLightCountsToShader(const Shader& shader) const
{
	shader.setUniformi("u_DirLightCount", DirectionalLight::m_lightCountByType);
	shader.setUniformi("u_PointLightCount", PointLight::m_lightCountByType);
	shader.setUniformi("u_SpotLightCount", SpotLight::m_lightCountByType);

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


