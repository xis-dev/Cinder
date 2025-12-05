#pragma once
#include <vector>

#include "Objects/Headers/EBO.h"
#include "Objects/Headers/VAO.h"
#include "Objects/Headers/VBO.h"
#include "Objects/General/Vertex.h"
#include "Objects/Headers/Shader.h"
#include "Objects/Headers/Texture.h"

class Mesh
{
public:

	Mesh() = default;

	Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices);


	std::vector<Vertex> m_vertices{};
	std::vector<unsigned int> m_indices{};
	std::vector<Texture> m_textures{};
	std::string tag{};
	unsigned int vao, vbo, ebo{};


	void setupMesh();

	void draw() const;


};
