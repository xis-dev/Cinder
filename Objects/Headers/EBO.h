#pragma once
#include <glad/glad.h>

class EBO
{
public:
	GLuint ID{};

	EBO() = default;
	EBO(GLsizei size, const void* data, GLenum usage = GL_STATIC_DRAW);

	// both not needed

	void bind();
	void unbind();
	~EBO();

};
