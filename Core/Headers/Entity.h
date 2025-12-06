#pragma once
#include <bitset>
#include <ranges>

#include <vector>
#include <tuple>
#include <unordered_map>

#include "Material.h"
#include "map"
#include "Mesh.h"
#include "Objects/Headers/Camera.h"

// New entity includes
#include "imgui.h"
#include <string>
#include <cmath>

class Scene;


class Entity 
{
	static constexpr size_t MAX_NAME_LENGTH = 32;

public:
	virtual ~Entity() = default;


protected:
	char m_tag[MAX_NAME_LENGTH];
	Vec3f m_position{};
	Vec3f m_currentRotationAxis{Vec3f(0.0f, 1.0f, 0.0f)};
	float m_currentRotationAngle{};

	std::shared_ptr<Texture> m_icon;
	bool m_hasIcon{};

public:

	Vec3f getPosition() { return m_position;}
	Vec3f getRotationAxis() { return m_currentRotationAxis; }
	float getRotationAngle() { return m_currentRotationAngle; }

	void setPosition(Vec3f pos) { m_position = pos; }
	void setPosition(float p) { m_position = Vec3f(p); }


	void setRotation(Vec3f axis, float angle)
	{
		m_currentRotationAxis = axis;
		m_currentRotationAngle = angle;
	}

	void setTag(const std::string& tag)
	{
		if (tag.length() > MAX_NAME_LENGTH)
		{
			std::cout << "ENTITY:: Attempting to name entity with more than " << MAX_NAME_LENGTH << " characters. Consider reducing name length.\n";
			std::string shortenedTag{ tag };
			shortenedTag.erase(MAX_NAME_LENGTH, std::string::npos);
			strcpy(m_tag, shortenedTag.c_str());
			return;
		}
		strcpy(m_tag, tag.c_str());
	}

	const char* getTag()
	{
		return m_tag;
	}

	virtual void setIcon(const std::shared_ptr<Texture>& icon)
	{
		m_hasIcon = true;
		m_icon = icon;
	}

	bool hasIcon()
	{
		return m_hasIcon;
	}

	const std::shared_ptr<Texture> getIcon()
	{
		return m_icon;
	}




	virtual void imguiDraw()
	{
		ImGui::InputText("Entity Name", m_tag, MAX_NAME_LENGTH);
		ImGui::DragFloat3("Position", &m_position.x, 0.5f);
		ImGui::DragFloat3("Rotation Axis", &m_currentRotationAxis.x, 0.1f);
		ImGui::DragFloat("Angle", &m_currentRotationAngle);
	}
	// TODO: In spirit of generalization, add gizmos(?)
	virtual glm::mat4 getTransformMatrix() {
		glm::mat4 modelMatrix{ 1.0f };
		modelMatrix = glm::translate(modelMatrix, static_cast<glm::vec3>(getPosition()));
		modelMatrix = glm::rotate(modelMatrix, glm::radians((getRotationAngle())), static_cast<glm::vec3>(getRotationAxis()));
		return modelMatrix;
	}


	virtual void render(const std::shared_ptr<Shader>& shader) { 
		
		shader->use();
		shader->setUniformMat4("u_ModelMatrix", getTransformMatrix()); 
	}
};

class MeshEntity : public Entity{
public:
	MeshEntity() = default;
	MeshEntity(const std::shared_ptr<Mesh> mesh, const std::shared_ptr<Material> material);


private:
	std::vector<std::shared_ptr<Mesh>> m_meshes{};
	std::vector<std::shared_ptr<Material>> m_materials{};

	Vec3f m_scale{1.0f};

public:
	Vec3f getScale() { return m_scale; }
	void setScale(Vec3f scale) { m_scale = scale; }
	void setScale(float s) { m_scale = Vec3f(s); }

	virtual void imguiDraw() override;
	virtual glm::mat4 getTransformMatrix() override;
	virtual void render(const std::shared_ptr<Shader>& shader) override;
};

class LightEntity : public Entity{
public:
	LightEntity(Vec3f color = Vec3f(1.0f), 
				float intensity = 1.0f):	m_color(color),
											m_intensity(intensity){}
											
	virtual ~LightEntity() = default;
protected:
	// Light ID in specific group, used for the index to send input to the light array in shaders
	int m_lightID{};
	Vec3f m_color{ 1.0f };
	float m_intensity{ 1.0f };

public:
	void setLightID(int id) { m_lightID = id; }
	void setColor(Vec3f color) { m_color = color; }
	Vec3f getColor() { return m_color; }

	virtual void imguiDraw() override;
	virtual void use(const std::shared_ptr<Shader>& shader) {};
};

class DirectionalLight : public LightEntity {
public:
	DirectionalLight(Vec3f direction = Vec3f(0.0f, 1.0f, 0.0f),
					 Vec3f color = Vec3f(1.0f), 
					 float intensity = 1.0f): LightEntity(color, intensity), 
											  m_direction(direction) {}
	Vec3f m_direction{};
	static int m_lightCountByType;

	virtual void imguiDraw() override;
	virtual void use(const std::shared_ptr<Shader>& shader) override;
	
};

class PointLight : public LightEntity {
public:
	PointLight(float attenuationRadius, 
			   Vec3f color = Vec3f(1.0f),
			   float intensity = 1.0f): LightEntity(color, intensity)
	{
		Vec3f attenuationValueSet = getAttenuationValues(attenuationRadius);
		m_constant = attenuationValueSet.x;
		m_linear = attenuationValueSet.y;
		m_quadratic = attenuationValueSet.z;
	}

	float m_radius{};
	float m_constant{};
	float m_linear{};
	float m_quadratic{};

	Vec3f getAttenuationValues(float radius, float threshold = 0.01f, float ratio = 1.0f, float constant = 1.0f)
	{
		float reqQuotient = (1.0f / threshold) - 1.0f;

		float quadratic = reqQuotient / ((ratio + 1.0f) * radius * radius);
		float linear = ratio * quadratic * radius;

		return Vec3f(constant, linear, quadratic);
	}

	static int m_lightCountByType;

	virtual void imguiDraw() override;
	virtual void use(const std::shared_ptr<Shader>& shader) override;
};

class SpotLight : public LightEntity {
public:
	SpotLight(Vec3f direction, float innerAngle, float outerAngle,
			  Vec3f color = Vec3f(1.0f), float intensity = 1.0f): LightEntity(color, intensity),
																  m_innerCutoff(innerAngle),
																  m_outerCutoff(outerAngle){}

	Vec3f m_direction{};
	float m_innerCutoff{};
	float m_outerCutoff;

	static int m_lightCountByType;

	virtual void imguiDraw() override;
	virtual void use(const std::shared_ptr<Shader>& shader) override;
};
