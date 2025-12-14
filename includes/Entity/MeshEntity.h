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

class MeshEntity : public Entity {
public:
	MeshEntity() = default;
	MeshEntity(Mesh* mesh);
	MeshEntity(std::vector<Mesh*>& meshes) : m_meshes(meshes) {}


private:
	std::vector<Mesh*> m_meshes{};

	Vec3f m_scale{ 1.0f };

public:
	Vec3f getScale() { return m_scale; }
	void setScale(Vec3f scale) { m_scale = scale; }
	void setScale(float s) { m_scale = Vec3f(s); }


	const std::vector<Mesh*>& getMeshes() const { return m_meshes; }

	virtual void imguiDraw() override;
	virtual glm::mat4 getTransformMatrix() override;
	virtual void render(const Shader& shader) override;
};