#pragma once
#include <vector>

#include "Graphics/Vertex.h"


#include "Resources/Resource.h"

class Shader;
class Material;

class Mesh: public Resource
{
public:

	Mesh() = default;

	Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, const std::vector<Material*>& materials);


	Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, Material* materials);

	std::vector<Material*> m_materials{};
	std::vector<Vertex> m_vertices{};
	std::vector<unsigned int> m_indices{};
	unsigned int vao{}, vbo{}, ebo{};

	const std::vector<Material*>& getMaterials() const { return m_materials; }


	void setupMesh();

	void draw(const Shader& shader) const;

	virtual void destroy() override {};

};
