#include "Objects/Headers/VAO.h"

#include "Objects/Headers/Vertex.h"



void VAO::init()
{
	glGenVertexArrays(1, &ID);
}

void VAO::linkAttrib(VBO& vbo, GLuint location, GLsizei size, GLint stride, GLint offset)
{
	vbo.bind();
	glVertexAttribPointer(location, size, GL_FLOAT, GL_FALSE, stride, (void*)(offset));
	glEnableVertexAttribArray(location);
}

void VAO::bind() const
{
	glBindVertexArray(ID);
}

void VAO::unbind() const
{
	glBindVertexArray(0);
}

VAO::~VAO()
{
	glDeleteVertexArrays(1, &ID);
}



