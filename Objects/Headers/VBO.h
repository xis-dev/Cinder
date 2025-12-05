#pragma once
#include <glad/glad.h>


class VBO
{

public:
	GLuint ID{};

	VBO() = default;
	~VBO();
	 VBO(GLsizei size, const void* data, GLenum usage = GL_STATIC_DRAW);

	void bind();
	void unbind();
};
