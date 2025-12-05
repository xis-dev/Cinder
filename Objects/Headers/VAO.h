#pragma once

#include <glad/glad.h>
#include "Objects/Headers/VBO.h"


class VAO
{
public:
	GLuint ID{};

	 VAO()= default;
	~VAO();

	void init();
	void linkAttrib(VBO& vbo,GLuint location, GLsizei size, GLint stride, GLint offset);

	void bind() const;

	void unbind() const;


};