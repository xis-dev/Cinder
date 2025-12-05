#pragma once
#include <string>
#include <vector>
#include <glad/glad.h>

class Shader;

class Texture
{
public:
	constexpr static unsigned MAX_UNITS{ 32 };

	GLuint ID{};
	std::string m_path{};
	enum Type
	{
		Diffuse,
		Specular
	};

	Type type{Diffuse};
	Texture() = default;

	Texture(GLuint id, std::string path, Type texType) : ID(id), m_path(std::move(path)), type(texType){}


	Texture(const char* fileName, Type texType = Type::Diffuse, bool flipOnLoad = true, GLenum wrapType = GL_REPEAT, GLuint desiredFormat = GL_RGBA);
	static unsigned int loadTextureFile(const char* filePath, Type texType = Type::Diffuse, bool flipOnLoad = true, GLenum wrapType = GL_REPEAT, GLuint desiredFormat = GL_RGBA);

	static GLuint loadTextureFile(const char* relativePath, const std::string& directory, Type texType = Type::Diffuse, bool flipOnLoad = true, GLenum wrapType = GL_REPEAT, GLuint desiredFormat = GL_RGBA);
	void bind();
	static void unbind(GLenum activeTexUnit);

	~Texture();

private:

	static void setWrap(GLenum wrapAxis, GLenum wrapType);

};
