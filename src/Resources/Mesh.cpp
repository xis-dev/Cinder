#include "Resources/Mesh.h"
#include "Resources/Shader.h"
#include "Resources/Texture.h"

#include "Material.h"

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices): m_vertices(vertices),
									  m_indices(indices)
{
	setupMesh();
}


void Mesh::setupMesh()
{
	if (m_vertices.empty() || m_indices.empty())
	{
		std::cout << "Empty Mesh cannot be instantiated. \n";
		return;
	}
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

		
	m_idxCount = m_indices.size();

	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * m_vertices.size(), m_vertices.data(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned) * m_indices.size(), m_indices.data(), GL_STATIC_DRAW);




	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, position)));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, texCoords)));
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, tangent)));
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, bitangent)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	m_vertices.clear();
	m_vertices.shrink_to_fit();

	m_indices.clear();
	m_indices.shrink_to_fit();

}

void Mesh::draw() const
{
	glBindVertexArray(vao);

	glDrawElements(GL_TRIANGLES, GLsizei(m_idxCount), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	
}

Mesh::~Mesh()
{


	if (vao != 0)
	{
		glDeleteVertexArrays(1, &vao);
	}
	if (vbo != 0)
	{
		glDeleteBuffers(1, &vbo);
	}
	if (ebo != 0)
	{
		glDeleteBuffers(1, &ebo);
	}

}

//void Mesh::destroy()
//{
//	std::cout << "Destroying Mesh; VAO: " << vao << ", VBO: " << ", EBO: " << ebo << "\n";
//	glDeleteVertexArrays(1, &vao);
//	glDeleteBuffers(1, &vbo);
//	glDeleteBuffers(1, &ebo);
//}





