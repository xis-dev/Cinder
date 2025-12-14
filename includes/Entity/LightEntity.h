#pragma once

#include "Entity.h"

class LightEntity : public Entity {
public:
	LightEntity(Vec3f color = Vec3f(1.0f),
		float intensity = 1.0f) : m_color(color),
		m_intensity(intensity)
	{
	}

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
	virtual void use(const Shader& shader) {};
};

class DirectionalLight : public LightEntity {
public:
	DirectionalLight(Vec3f direction = Vec3f(0.0f, 1.0f, 0.0f),
		Vec3f color = Vec3f(1.0f),
		float intensity = 1.0f) : LightEntity(color, intensity),
		m_direction(direction)
	{
	}
	Vec3f m_direction{};
	static int m_lightCountByType;

	virtual void imguiDraw() override;
	virtual void use(const Shader& shader) override;

};

class PointLight : public LightEntity {
public:
	PointLight(float attenuationRadius,
		Vec3f color = Vec3f(1.0f),
		float intensity = 1.0f) : LightEntity(color, intensity)
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
	virtual void use(const Shader& shader) override;
};

class SpotLight : public LightEntity {
public:
	SpotLight(Vec3f direction, float innerAngle, float outerAngle,
		Vec3f color = Vec3f(1.0f), float intensity = 1.0f) : LightEntity(color, intensity),
		m_innerCutoff(innerAngle),
		m_outerCutoff(outerAngle)
	{
	}

	Vec3f m_direction{};
	float m_innerCutoff{};
	float m_outerCutoff;

	static int m_lightCountByType;

	virtual void imguiDraw() override;
	virtual void use(const Shader& shader) override;
};
