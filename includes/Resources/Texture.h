#pragma once

#include "Resources/Resource.h"

#include <string>
#include <vector>

#include "glad/glad.h"


class Texture: public Resource
{
private:
	constexpr static unsigned MAX_UNITS{ 32 };

	unsigned m_id{};
	std::string m_location{};
	


public:
	enum Type {
		Diffuse,
		Specular
	};

private:
	Type m_type{ Diffuse };

public:
	/*Texture(const Texture& t) = delete;
	Texture& operator=(const Texture& t) = delete;

	Texture(Texture&& t)noexcept : m_id(t.m_id)
	{
		t.m_id = 0;
	}

	Texture& operator=(Texture&& t) noexcept
	{
		if (this != &t)
		{
			m_id = t.m_id;
			t.m_id = 0;
		}
		return *this;
	}*/

	Texture() = default;


	Texture(const std::string& fileName, Type texType = Type::Diffuse, bool flipOnLoad = true, GLenum wrapType = GL_REPEAT, unsigned desiredFormat = GL_RGBA);

	static unsigned loadTextureFile(const std::string& filePath, Type texType = Type::Diffuse, bool flipOnLoad = true, GLenum wrapType = GL_REPEAT, unsigned desiredFormat = GL_RGBA);

	Type getType();
	const std::string& getLocation()
	{
		return m_location;
	}

	 void use() const;
	static void unbind(GLenum activeTexUnit);

	virtual void destroy() override;

private:

	static void setWrap(GLenum wrapAxis, GLenum wrapType);

};
