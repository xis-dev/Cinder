#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/matrix.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
private:
	enum ObjectType
	{
		Program, Vertex, Fragment
	};
	void errorCheck(GLuint object, ObjectType type);

public:
	GLuint ID;
	Shader(const char* vertexFile, const char* fragmentFile);
	std::string getShaderSource(const char* fileName);
	void setUniformi(const char* name, int value) const;
	void setUniformf(const char* name, float value) const;
	void setUniformMat4(const char* name, glm::mat4 value) const;
	void setUniformVec3(const char* name, glm::vec3 value) const;
	void use() const;

	static std::shared_ptr<Shader>& getDefaultShader()
	{
		static std::shared_ptr<Shader> defaultShader = std::make_shared<Shader>("Shaders/default.vert", "Shaders/default_lit.frag");
		return defaultShader;
	}

	static std::shared_ptr<Shader>& getTexturedShader()
	{
		static std::shared_ptr<Shader> shader = std::make_shared<Shader>("Shaders/default.vert", "Shaders/textured_lit.frag");
		return shader;
	}

	~Shader();

};
