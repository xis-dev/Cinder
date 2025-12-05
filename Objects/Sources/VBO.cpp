#include "Objects/Headers/VBO.h"


VBO::VBO(GLsizei size, const void* data, GLenum usage)
{
	glGenBuffers(1, &ID);
	glBindBuffer(GL_ARRAY_BUFFER, ID);

	glBufferData(GL_ARRAY_BUFFER, size, data, usage);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::bind()
{
	glBindBuffer(GL_ARRAY_BUFFER, ID);
}

void VBO::unbind()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

VBO::~VBO()
{
	glDeleteBuffers(1, &ID);
}




