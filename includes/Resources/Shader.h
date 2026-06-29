#pragma once


#include "Resources/Resource.h"
#include <ext/matrix_float4x4.hpp>
#include <ext/vector_float3.hpp>

#include <unordered_map>
#include <initializer_list>
#include <string>


class Shader: public Resource
{
private:
	enum ObjectType
	{
		Program, Vertex, Fragment, Geometry
	};

	unsigned m_id = 0;
	void errorCheck(unsigned object, ObjectType type);
	std::string getShaderSource(const char* fileName);

public:
	mutable std::unordered_map<std::string, int> uniformLocations;

	/*Shader(const Shader& s) = delete;
	Shader& operator=(const Shader& s) = delete;

	Shader(Shader&& s) noexcept: m_id(s.m_id)
	{
		s.m_id = 0;
	}

	Shader& operator=(Shader&& s) noexcept
	{
		if (this != &s)
		{
			m_id = s.m_id;
			s.m_id = 0;
		}
		return *this;
	}*/

	Shader(const char* vertexFile, const char* fragmentFile);
	Shader(const char* vertexFile, const char* fragmentFile, const char* geometryFile);
	void setUniformi(const char* name, int value) const;
	void setUniformf(const char* name, float value) const;
	void setUniformMat4(const char* name, glm::mat4 value) const;
	void setUniformVec3(const char* name, glm::vec3 value) const;
	void setUniformVec3Array(const char* name, const glm::vec3* data, int count) const;
	void use() const;


	virtual void destroy() override;


};
