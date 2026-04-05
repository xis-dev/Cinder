#pragma once

#include "Entity.h"
#include "Math/Vec3.h"

#include "glm/matrix.hpp"

#include <vector>

struct aiNode;
struct aiScene;
struct aiMesh;

class Mesh;
class Material;
class Shader;
class Model;

class MeshEntity : public Entity {
public:
	MeshEntity() = default;
	MeshEntity(Model* model) : m_model(model) {}


private:
	Model* m_model{};

	glm::vec3 m_scale{ 1.0f };

public:
	glm::vec3 getScale() { return m_scale; }
	void setScale(glm::vec3 scale) { m_scale = scale; }
	void setScale(float s) { m_scale = glm::vec3(s); }


	Model* getModel() { return m_model; }
	virtual void imguiDraw() override;
	virtual glm::mat4 getTransformMatrix() override;
};