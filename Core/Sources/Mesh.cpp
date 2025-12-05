#include "Core/Headers/Mesh.h"

#include <iostream>


Mesh::Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices)
{
	m_vertices = vertices;
	m_indices = indices;

	setupMesh();
}


void Mesh::setupMesh()
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	if (!m_vertices.empty()) glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * m_vertices.size(), m_vertices.data(), GL_STATIC_DRAW);
	if (!m_indices.empty()) glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * m_indices.size(), m_indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, m_Position)));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, m_Normal)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, m_TexCoords)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);

}

// Call after material setup
void Mesh::draw() const
{
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}


