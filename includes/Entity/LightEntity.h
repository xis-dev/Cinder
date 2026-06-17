#pragma once

#include "Entity.h"

class LightEntity : public Entity {
public:
	LightEntity(glm::vec3 color = glm::vec3(1.0f),
		float intensity = 1.0f) : m_color(color),
		m_intensity(intensity)
	{
	}

	virtual ~LightEntity() = default;
protected:
	// Light ID in specific group, used for the index to send input to the light array in shaders
	int m_lightID{};
	glm::vec3 m_color{ 1.0f };
	float m_intensity{ 1.0f };

public:

	bool canCastShadow{true};
	void setLightID(int id) { m_lightID = id; }
	void setIntensity(float i) { m_intensity = i; }
	float getIntensity() { return m_intensity; }
	void setColor(glm::vec3 color) { m_color = color; }
	glm::vec3 getColor() { return m_color; }

	virtual void imguiDraw() override;
	virtual void use(const Shader& shader) {};
};

class DirectionalLight : public LightEntity {
public:
	DirectionalLight(glm::vec3 direction = glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3 color = glm::vec3(1.0f),
		float intensity = 1.0f) : LightEntity(color, intensity),
		m_direction(direction)
	{
	}
	glm::vec3 m_direction{};
	static int m_lightCountByType;

	virtual void imguiDraw() override;
	virtual void use(const Shader& shader) override;

};

class PointLight : public LightEntity {
public:
	PointLight(float attenuationRadius,
		glm::vec3 color = glm::vec3(1.0f),
		float intensity = 1.0f, float castShadow = true) : LightEntity(color, intensity)
	{
		m_radius = attenuationRadius;
		glm::vec3 attenuationValueSet = getAttenuationValues(attenuationRadius);
		m_constant = attenuationValueSet.x;
		m_linear = attenuationValueSet.y;
		m_quadratic = attenuationValueSet.z;
	}

	float m_radius{};
	float m_constant{};
	float m_linear{};
	float m_quadratic{};

	// Point Map FBO Attachment
	unsigned shadowMap;

	void initShadowMap()
	{

	}

	glm::vec3 getAttenuationValues(float radius, float threshold = 0.01f, float ratio = 1.0f, float constant = 1.0f)
	{
		float reqQuotient = (1.0f / threshold) - 1.0f;

		float quadratic = reqQuotient / ((ratio + 1.0f) * radius * radius);
		float linear = ratio * quadratic * radius;

		return glm::vec3(constant, linear, quadratic);
	}

	static int m_lightCountByType;

	virtual void imguiDraw() override;
	virtual void use(const Shader& shader) override;
};

class SpotLight : public LightEntity {
public:
	SpotLight(glm::vec3 direction, float innerAngle, float outerAngle,
		glm::vec3 color = glm::vec3(1.0f), float intensity = 1.0f) : LightEntity(color, intensity),
		m_innerCutoff(innerAngle),
		m_outerCutoff(outerAngle)
	{
	}

	glm::vec3 m_direction{};
	float m_innerCutoff{};
	float m_outerCutoff;

	static int m_lightCountByType;

	virtual void imguiDraw() override;
	virtual void use(const Shader& shader) override;
};
