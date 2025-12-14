#include "Entity/LightEntity.h"

#include "Shader.h"
#include "imgui.h"

void DirectionalLight::imguiDraw()
{
	LightEntity::imguiDraw();
	ImGui::DragFloat3("Light Direction", &m_direction.x);
}

void DirectionalLight::use(const Shader& shader)
{
	LightEntity::use(shader);
	const std::string uniformStr{ "u_DirectionalLights[" + std::to_string(m_lightID) + "]." };

	shader.setUniformVec3((uniformStr + "direction").c_str(), m_direction.getNormalized());
	shader.setUniformVec3((uniformStr + "color").c_str(), m_color);
	shader.setUniformf((uniformStr + "intensity").c_str(), m_intensity);
}

int DirectionalLight::m_lightCountByType{};
int SpotLight::m_lightCountByType{};
int PointLight::m_lightCountByType{};


void LightEntity::imguiDraw()
{
	Entity::imguiDraw();
	ImGui::ColorEdit3("Light Color", &m_color.x);
	ImGui::DragFloat("Intensity", &m_intensity, 0.5f);
}

void PointLight::imguiDraw()
{
	LightEntity::imguiDraw();
	if (ImGui::DragFloat("Attenuation Radius", &m_radius))
	{
		Vec3f attenuationValueSet = getAttenuationValues(m_radius);
		m_constant = attenuationValueSet.x;
		m_linear = attenuationValueSet.y;
		m_quadratic = attenuationValueSet.z;
	}
}

void PointLight::use(const Shader& shader)
{
	LightEntity::use(shader);
	const std::string uniformStr{ "u_PointLights[" + std::to_string(m_lightID) + "]." };

	shader.setUniformf((uniformStr + "constant").c_str(), m_constant);
	shader.setUniformf((uniformStr + "linear").c_str(), m_linear);
	shader.setUniformf((uniformStr + "quadratic").c_str(), m_quadratic);
	shader.setUniformf((uniformStr + "intensity").c_str(), m_intensity);

	shader.setUniformVec3((uniformStr + "color").c_str(), m_color);
	shader.setUniformVec3((uniformStr + "position").c_str(), getPosition());
}


void SpotLight::imguiDraw()
{
	LightEntity::imguiDraw();
	ImGui::DragFloat("Inner Radius", &m_innerCutoff);
	ImGui::DragFloat("Outer Radius", &m_outerCutoff);

}

void SpotLight::use(const Shader& shader)
{
	LightEntity::use(shader);
	const std::string uniformStr = "u_SpotLights[" + std::to_string(m_lightID) + "].";


	shader.setUniformf((uniformStr + "innerCutoff").c_str(), m_innerCutoff);
	shader.setUniformf((uniformStr + "outerCutoff").c_str(), m_outerCutoff);
	shader.setUniformVec3((uniformStr + "color").c_str(), m_color);
	shader.setUniformVec3((uniformStr + "direction").c_str(), m_direction.getNormalized());
	shader.setUniformVec3((uniformStr + "position").c_str(), getPosition());
	shader.setUniformf((uniformStr + "intensity").c_str(), m_intensity);

}
