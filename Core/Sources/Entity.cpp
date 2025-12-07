#include "Core/Headers/Entity.h"



MeshEntity::MeshEntity(Mesh& mesh, Material& material)
{
	m_meshes.push_back(&mesh);
	m_materials.push_back(&material);
}

void MeshEntity::imguiDraw()
{
	Entity::imguiDraw();

	ImGui::DragFloat3("Scale", &m_scale.x);
}

glm::mat4 MeshEntity::getTransformMatrix()
{
	glm::mat4 modelMatrix{ 1.0f };
	modelMatrix = glm::translate(modelMatrix, static_cast<glm::vec3>(getPosition()));
	modelMatrix = glm::rotate(modelMatrix, glm::radians((getRotationAngle())), static_cast<glm::vec3>(getRotationAxis()));
	modelMatrix = glm::scale(modelMatrix, static_cast<glm::vec3>(getScale()));
	return modelMatrix;
}

void MeshEntity::render(const Shader& shader)
{
	Entity::render(shader);
	for (auto& mat : m_materials)
	{
		mat->use();
	}
	
	for (auto& mesh : m_meshes)
	{
		mesh->use();
	}
}

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
