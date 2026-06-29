#include "Resources/Shader.h"
#include "Utilities/FileManager.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"




Shader::Shader(const char* vertexFile, const char* fragmentFile)
{
	unsigned vertex = glCreateShader(GL_VERTEX_SHADER);
	unsigned fragment = glCreateShader(GL_FRAGMENT_SHADER);

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

	m_id = glCreateProgram();
	glAttachShader(m_id, vertex);
	glAttachShader(m_id, fragment);

	glLinkProgram(m_id);
	errorCheck(m_id, ObjectType::Program);

	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

Shader::Shader(const char* vertexFile, const char* fragmentFile, const char* geometryFile)
{
	unsigned vertex = glCreateShader(GL_VERTEX_SHADER);
	unsigned fragment = glCreateShader(GL_FRAGMENT_SHADER);
	unsigned geometry = glCreateShader(GL_GEOMETRY_SHADER);

	std::string vertexCode = getShaderSource(vertexFile);
	std::string fragmentCode = getShaderSource(fragmentFile);
	std::string geometryCode = getShaderSource(geometryFile);

	const char* vertexSource = vertexCode.c_str();
	const char* fragmentSource = fragmentCode.c_str();
	const char* geometrySource = geometryCode.c_str();

	glShaderSource(vertex, 1, &vertexSource, nullptr);
	glShaderSource(fragment, 1, &fragmentSource, nullptr);
	glShaderSource(geometry, 1, &geometrySource, nullptr);

	glCompileShader(vertex);
	errorCheck(vertex, Vertex);

	glCompileShader(fragment);
	errorCheck(fragment, Fragment);
	
	glCompileShader(geometry);
	errorCheck(geometry, Geometry);

	m_id = glCreateProgram();
	glAttachShader(m_id, vertex);
	glAttachShader(m_id, fragment);
	glAttachShader(m_id, geometry);

	glLinkProgram(m_id);
	errorCheck(m_id, ObjectType::Program);

	glDeleteShader(vertex);
	glDeleteShader(fragment);
	glDeleteShader(geometry);
}

std::string Shader::getShaderSource(const char* fileName)
{
	std::ifstream stream(FileManager::getPath(fileName));
	stream.open(FileManager::getPath(fileName));
	if (!stream.is_open())
	{
		std::cout << "FAILED TO OPEN FILE. filename: " << fileName << "\n";
		return "";
	}
	std::stringstream string;
	string << stream.rdbuf();
	return string.str();
}

void Shader::setUniformi(const char* name, int value) const
{
	auto uniformSet = uniformLocations.find(name);
	GLint uniformLoc;
	if (uniformSet != uniformLocations.end())
	{
		uniformLoc = uniformSet->second;
	}
	else
	{
		uniformLoc = glGetUniformLocation(m_id, name);
		uniformLocations.insert({name, uniformLoc});
	}
	glUniform1i(uniformLoc, value);
}

void Shader::setUniformf(const char* name, float value) const
{
	auto uniformSet = uniformLocations.find(name);
	GLint uniformLoc;
	if (uniformSet != uniformLocations.end())
	{
		uniformLoc = uniformSet->second;
	}
	else
	{
		uniformLoc = glGetUniformLocation(m_id, name);
		uniformLocations.insert({name, uniformLoc});
	}
	glUniform1f(uniformLoc, value);
}

void Shader::setUniformMat4(const char* name, glm::mat4 value) const
{
	auto uniformSet = uniformLocations.find(name);
	GLint uniformLoc;
	if (uniformSet != uniformLocations.end())
	{
		uniformLoc = uniformSet->second;
	}
	else
	{
		uniformLoc = glGetUniformLocation(m_id, name);
		uniformLocations.insert({name, uniformLoc});
	}
	glUniformMatrix4fv(uniformLoc, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::setUniformVec3(const char* name, glm::vec3 value) const
{
	auto uniformSet = uniformLocations.find(name);
	GLint uniformLoc;
	if (uniformSet != uniformLocations.end())
	{
		uniformLoc = uniformSet->second;
	}
	else
	{
		uniformLoc = glGetUniformLocation(m_id, name);
		uniformLocations.insert({name, uniformLoc});
	}
	glUniform3fv(uniformLoc, 1, glm::value_ptr(value));
}

void Shader::setUniformVec3Array(const char* name, const glm::vec3* data, int count) const {
	auto uniformSet = uniformLocations.find(name);
	GLint uniformLoc;
	if (uniformSet != uniformLocations.end())
	{
		uniformLoc = uniformSet->second;
	}
	else
	{
		uniformLocations.insert({name, uniformLoc});
	}
	glUniform3fv(uniformLoc, count, glm::value_ptr(data[0]));
}

void Shader::use() const
{
	glUseProgram(m_id);
}



void Shader::errorCheck(unsigned object, ObjectType type)
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
		case Geometry:
			glGetShaderiv(object, GL_COMPILE_STATUS, &success);
			if (!success)
			{
				glGetShaderInfoLog(object, 512, nullptr, infoLog);
				std::cout << "ERROR COMPILING GEOMETRY SHADER. \n" << infoLog;
			}
			break;
			break;
	}

}

void Shader::destroy()
{
	if (m_id != 0)
	{
		glDeleteProgram(m_id);

	}
	std::cout << "Deleting shader program: " << m_id << std::endl;
}

