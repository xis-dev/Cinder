#pragma once


#include "Math/Vec3.h"
#include "Delegate.h"
#include "glm/ext/matrix_transform.hpp"
#include <string>
#include <vector>

class Scene;

class Shader;
class Texture;


class Entity 
{
	static constexpr size_t MAX_NAME_LENGTH = 32;
	
public:
	virtual ~Entity() = default;

private:
	Entity*m_parent = nullptr;
	std::vector<Entity*> m_children;

protected:
	char m_tag[MAX_NAME_LENGTH]{};
	glm::vec3 m_position{};
	glm::vec3 m_currentRotationAxis{glm::vec3(0.0f, 1.0f, 0.0f)};
	float m_currentRotationAngle{};

	Texture* m_icon{};
	bool m_hasIcon{};

	// Returns success state boolean
	void findAndRemoveChild(Entity* child);
public:

	void setParent(Entity* child);

	std::vector<Entity*> getChildren() const;
	glm::vec3 getRelativePosition() const { return m_position;}
	glm::vec3 getWorldPosition() const;
	glm::vec3 getRelativeRotationAxis() const { return m_currentRotationAxis; }
	float getRelativeRotationAngle() const{ return m_currentRotationAngle; }

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

	virtual glm::mat4 getRelativeTransformMatrix();

	// Transform matrix globally after parent transformation
	virtual glm::mat4 getGlobalTransformMatrix();
};



