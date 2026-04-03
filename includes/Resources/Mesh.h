#pragma once

#include "Graphics/Vertex.h"


#include "Resources/Resource.h"

#include <vector>
#include <memory>

class Shader;
class Material;

class Mesh
{
public:

	Mesh() = default;

	Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices);


	std::vector<Vertex> m_vertices{};
	std::vector<unsigned int> m_indices{};
	unsigned int vao{}, vbo{}, ebo{};


	
	void setupMesh();

	void draw() const;

	void destroy();

};
