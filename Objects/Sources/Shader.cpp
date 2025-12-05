#include "Objects/Headers/Shader.h"

#include <GLFW/glfw3native.h>

#include "Core/FileManager.h"


Shader::Shader(const char* vertexFile, const char* fragmentFile)
{
	GLuint vertex, fragment;

	vertex  = glCreateShader(GL_VERTEX_SHADER);
	fragment = glCreateShader(GL_FRAGMENT_SHADER);

	std::string vertexCode = getShaderSource(vertexFile);
	std::string fragmentCode = getShaderSource(fragmentFile);

	const char* vertexSource = vertexCode.c_str();
	const char* fragmentSource = fragmentCode.c_str();

	glShaderSource(vertex, 1, &vertexSource, nullptr);
	glShaderSource(fragment, 1, &fragmentSource, nullptr);

	glCompileShader(vertex);
	errorCheck(vertex, ObjectType::Vertex);
	glCompileShader(fragment);
	errorCheck(fragment, ObjectType::Fragment);

	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);

	glLinkProgram(ID);
	errorCheck(ID, ObjectType::Program);

	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

std::string Shader::getShaderSource(const char* fileName)
{
	std::ifstream stream(FileManager::getPath(fileName));
	stream.open(FileManager::getPath(fileName));
	if (!stream.is_open())
	{
		std::cout << "FAILED TO OPEN FILE. filename: " << fileName << "\n";
		return nullptr;
	}
	std::stringstream string;
	string << stream.rdbuf();
	return string.str();
}

void Shader::setUniformi(const char* name, int value) const
{
	auto uniformLoc = glGetUniformLocation(ID, name);
	glUseProgram(ID);
	glUniform1i(uniformLoc, value);
}

void Shader::setUniformf(const char* name, float value) const
{
	auto uniformLoc = glGetUniformLocation(ID, name);
	glUseProgram(ID);
	glUniform1f(uniformLoc, value);
}

void Shader::setUniformMat4(const char* name, glm::mat4 value) const
{
	auto uniformLoc = glGetUniformLocation(ID, name);
	glUseProgram(ID);
	glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setUniformVec3(const char* name, glm::vec3 value) const
{
	auto uniformLoc = glGetUniformLocation(ID, name);
	glUniform3fv(uniformLoc, 1, glm::value_ptr(value));
}


void Shader::use() const
{
	glUseProgram(ID);
}

Shader::~Shader()
{
	std::cout << "Deleting shader program: " << ID << std::endl;
	glDeleteProgram(ID);
}



void Shader::errorCheck(GLuint object, ObjectType type)
{
	int success;
	char infoLog[512];

	switch (type) // bad solution? will not need more than these 3 anytime soon.
	{
		case ObjectType::Program:
			glGetProgramiv(object, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(object, 512, nullptr, infoLog);
				std::cout << "ERROR lINKING SHADER PROGRAM. \n" << infoLog;
			}
		break;

		case ObjectType::Vertex:
			glGetShaderiv(object, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(object, 512, nullptr, infoLog);
				std::cout << "ERROR COMPILING VERTEX SHADER. \n" << infoLog;
			}
		break;

		case ObjectType::Fragment:
			glGetShaderiv(object, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(object, 512, nullptr, infoLog);
				std::cout << "ERROR COMPILING FRAGMENT SHADER. \n" << infoLog;
			}
			break;
	}

}
