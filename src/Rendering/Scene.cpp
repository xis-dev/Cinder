#include "Scene.h"
#include "Resources/Shader.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/matrix.hpp>
#include <glm/ext/matrix_clip_space.hpp>


void Scene::init(AssetManager* asset_manager)
{
	ASSET_MANAGER = asset_manager;
}

void Scene::applyLightCountsToShader(const Shader& shader) const
{
	shader.setUniformi("u_DirLightCount", DirectionalLight::m_lightCountByType);
	shader.setUniformi("u_PointLightCount", PointLight::m_lightCountByType);
	shader.setUniformi("u_SpotLightCount", SpotLight::m_lightCountByType);

}

void Scene::setupPointMatrices(int w, const int h)
{
	for (auto& ps: m_pointShadows)
	{
		const auto* light = ps.first;
		auto&[shadowCubemap, shadowMapTransforms] = ps.second;
		shadowMapTransforms.clear();
		float aspect = (float)w / (float)h;
		float near = 1.0f;
		glm::mat4 shadow_proj = glm::perspective(glm::radians(90.0f), aspect, near, light->m_radius);

		glm::vec3 lightPos = light->getRelativePosition();
		shadowMapTransforms.push_back(shadow_proj *
			glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		shadowMapTransforms.push_back(shadow_proj *
			glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0)));
		shadowMapTransforms.push_back(shadow_proj *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
		shadowMapTransforms.push_back(shadow_proj *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0)));
		shadowMapTransforms.push_back(shadow_proj *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0)));
		shadowMapTransforms.push_back(shadow_proj *
			glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0)));
	}


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


