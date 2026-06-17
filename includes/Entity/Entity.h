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
	glm::vec3 m_position{};
	glm::vec3 m_currentRotationAxis{glm::vec3(0.0f, 1.0f, 0.0f)};
	float m_currentRotationAngle{};

	Texture* m_icon{};
	bool m_hasIcon{};

public:

	glm::vec3 getPosition() const { return m_position;}
	glm::vec3 getRotationAxis() const { return m_currentRotationAxis; }
	float getRotationAngle() const{ return m_currentRotationAngle; }

	void setPosition(glm::vec3 pos) { m_position = pos; }
	void setPosition(float p) { m_position = glm::vec3(p); }


	void setRotation(glm::vec3 axis, float angle);

	void setTag(const std::string& tag);

	const char* getTag() const;

	virtual void setIcon(Texture& icon);

	bool hasIcon();

	Texture* tryGetIcon();

	const Texture* tryGetIcon() const;

	virtual void imguiDraw();

	virtual glm::mat4 getTransformMatrix();
};



