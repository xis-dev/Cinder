#include "Objects/Headers/EBO.h"

EBO::EBO(GLsizei size, const void* data, GLenum usage)
{
	glGenBuffers(1, &ID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, usage);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void EBO::bind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
}

void EBO::unbind()
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

EBO::~EBO()
{
	glDeleteBuffers(1, &ID);
}





