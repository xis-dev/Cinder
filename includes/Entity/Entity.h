#pragma once


#include "Math/Vec3.h"
#include "glm/ext/matrix_transform.hpp"
#include <string>

class Scene;

class Shader;
class Texture;


class Entity 
{
	static constexpr size_t MAX_NAME_LENGTH = 32;
	
public:
	virtual ~Entity() = default;


protected:
	char m_tag[MAX_NAME_LENGTH]{};
	Vec3f m_position{};
	Vec3f m_currentRotationAxis{Vec3f(0.0f, 1.0f, 0.0f)};
	float m_currentRotationAngle{};

	Texture* m_icon{};
	bool m_hasIcon{};

public:

	Vec3f getPosition() { return m_position;}
	Vec3f getRotationAxis() { return m_currentRotationAxis; }
	float getRotationAngle() { return m_currentRotationAngle; }

	void setPosition(Vec3f pos) { m_position = pos; }
	void setPosition(float p) { m_position = Vec3f(p); }


	void setRotation(Vec3f axis, float angle);

	void setTag(const std::string& tag);

	const char* getTag();

	virtual void setIcon(Texture& icon);

	bool hasIcon();

	Texture* tryGetIcon();

	const Texture* tryGetIcon() const;

	virtual void imguiDraw();

	virtual glm::mat4 getTransformMatrix();


	virtual void render(const Shader& shader);
};



