#include "Resources/Texture.h"
#include "Utilities/FileManager.h"

#include "stb/stb_image.h"




Texture::Texture(const std::string& fileName , Texture::Type texType, bool flipOnLoad,
                 GLenum wrapType,unsigned desiredFormat): m_type(texType)
{
	m_id = loadTextureFile(fileName, texType, flipOnLoad, wrapType, desiredFormat);
	m_location = FileManager::getCanonicalPath(fileName);
}

Texture::Texture(const std::vector<std::string> cubeFaces)
{
	m_type = Cubemap;
	glGenTextures(1, &m_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);


	for (unsigned int i = 0; i < cubeFaces.size(); i++)
	{
		int width, height, nrColCh;
		GLenum imageFormat;
		switch (nrColCh)
		{
		case 1:
			imageFormat = GL_RED;
			break;
		case 2:
			imageFormat = GL_RG;
			break;
		case 4:
			imageFormat = GL_RGBA;
			break;
		default:
			imageFormat = GL_RGB;
		}
		unsigned char* data;
		data = stbi_load(FileManager::getPath(cubeFaces[i]).c_str(), &width, &height, &nrColCh, 0);
		if (data)
		{
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGBA, width, height, 0, imageFormat, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap tex failed to load at path: " << cubeFaces[i] << std::endl;
			stbi_image_free(data);
		}
		


	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}


unsigned int Texture::loadTextureFile(const std::string& filePath, Type texType, bool flipOnLoad, GLenum wrapType,
	unsigned desiredFormat)
{
	unsigned int id{};
	glGenTextures(1, &id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, id);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	setWrap(GL_TEXTURE_WRAP_R, wrapType);
	setWrap(GL_TEXTURE_WRAP_S, wrapType);
	setWrap(GL_TEXTURE_WRAP_T, wrapType);

	stbi_set_flip_vertically_on_load(flipOnLoad);
	int width, height, nrColCh;
	unsigned char* data = stbi_load(FileManager::getPath(filePath).c_str(), &width, &height, &nrColCh, 0);

	if (!data)
	{
		std::cout << "Failed to load texture: " << filePath << std::endl;
		std::cout << "Full path: " << FileManager::getPath(filePath) << std::endl;
		return id; 
	}


	GLenum imageFormat{};

	switch (nrColCh)
	{
		case 1:
			imageFormat = GL_RED;
			break;
		case 2:
			imageFormat = GL_RG;
			break;
		case 4:
			imageFormat = GL_RGBA;
			break;
		default:
			imageFormat = GL_RGB;
	}


	// 6407 = GL_RGB, 6408 = GL_RGBA

	glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLint>(desiredFormat), width, height, 0, imageFormat, GL_UNSIGNED_BYTE, data);

	glGenerateMipmap(GL_TEXTURE_2D);

	stbi_image_free(data);

	stbi_set_flip_vertically_on_load(false);

	glBindTexture(GL_TEXTURE_2D, 0);
	return id;
}

Texture::Type Texture::getType() const
{
	return m_type;
}

void Texture::setWrap(GLenum wrapAxis, GLenum wrapType)
{
	glTexParameteri(GL_TEXTURE_2D, wrapAxis, wrapType);
}


void Texture::use() const
{
	glBindTexture(GL_TEXTURE_2D, m_id);
}



void Texture::unbind(GLenum activeTexUnit) 
{
	if ((activeTexUnit < GL_TEXTURE0 || activeTexUnit > GL_TEXTURE0 + (MAX_UNITS - 1)))
	{
		std::cout << "TEXTURE:: Active texture input is out of range. There can only be " << MAX_UNITS << " texture units used by fragment shader.\n";
		return;
	}
	glActiveTexture(activeTexUnit);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::destroy()
{
	std::cout << "Destroying Texture: " << m_id << std::endl;
	glDeleteTextures(1, &m_id);
}

