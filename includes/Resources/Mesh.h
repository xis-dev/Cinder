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

	Mesh& operator=(const Mesh& mesh) = delete;
	Mesh(const Mesh& mesh) = delete;

	Mesh(Mesh&& m) noexcept: vao(m.vao), vbo(m.vbo), ebo(m.ebo),
							m_vertices(std::move(m.m_vertices)), m_indices(std::move(m.m_indices)), 
							m_idxCount(m.m_idxCount)
	{
		m.vao = 0;
		m.vbo = 0;
		m.ebo = 0;

		m.m_vertices.clear();
		m.m_indices.clear();

		m.m_idxCount = 0;

	}
	Mesh& operator=(Mesh&& m) noexcept {
		if (this != &m)
		{
			vao = m.vao;
			vbo = m.vbo;
			ebo = m.ebo;

			m.vao = 0;
			m.vbo = 0;
			m.ebo = 0;

			m_vertices = std::move(m.m_vertices);
			m_indices = std::move(m.m_indices);
			m_idxCount = m.m_idxCount;

			m.m_vertices.clear();
			m.m_indices.clear();
			m.m_idxCount = 0;
		}
		return *this;
	}

	unsigned int vao{}, vbo{}, ebo{};
	std::vector<Vertex> m_vertices{};
	std::vector<unsigned int> m_indices{};
	size_t m_idxCount;

	
	void setupMesh();

	void draw() const;

	~Mesh();

	//void destroy() override;


};
