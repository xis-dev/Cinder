#include "Resources/Material.h"
#include <Resources/Shader.h>
#include <Resources/Texture.h>



void Material::use(const Shader& shader) const
{
	if (m_textures.size() > 0)
	{
		int diffuseNr = 0;
		int specularNr = 0;
		int textureUnit = 0;

		for (int i = 0; i < (int)m_textures.size(); ++i)
		{
			auto& currentTex = m_textures[i];
			std::string uniformStr;

			switch (currentTex->getType())
			{

			case Texture::Specular:
				uniformStr = "t_Specular[" + std::to_string(specularNr) + "]";
				++specularNr;
				break;

			case Texture::Diffuse:

			default:
				uniformStr = "t_Diffuse[" + std::to_string(diffuseNr) + "]";
				++diffuseNr;
				break;
			}
			shader.setUniformi(uniformStr.c_str(), textureUnit);

			glActiveTexture(GL_TEXTURE0 + textureUnit);
			currentTex->use();
			++textureUnit;
		}

		shader.setUniformi("u_DiffuseMapCount", diffuseNr);
		shader.setUniformi("u_SpecularMapCount", specularNr);

		glActiveTexture(GL_TEXTURE0);
	}

	
	std::string materialUniformBase = "u_Material.";
	shader.setUniformVec3((materialUniformBase + "albedo").c_str(), m_baseColor);
	shader.setUniformf((materialUniformBase + "ambient").c_str(), m_ambientStrength);
	shader.setUniformf((materialUniformBase + "diffuse").c_str(), m_diffuseStrength);
	shader.setUniformf((materialUniformBase + "specular").c_str(), m_specularStrength);
	shader.setUniformf((materialUniformBase + "shininess").c_str(), m_shininess);


}

void Material::addTexture(Texture* texture)
{
	if (texture)
	{
		m_textures.push_back(texture);
	}
}





void Material::useShader() const
{
	if (m_shader)
	{
		m_shader->use();
	}
}

Shader* Material::tryGetShader()
{
	return m_shader;
}

const Shader* Material::tryGetShader() const
{
	return m_shader;
}

void Material::setColor(Vec3f color)
{
	m_baseColor = color;
}

void Material::setColor(float r, float g, float b)
{
	m_baseColor = Vec3f(r,g,b);
}

Vec3f Material::getColor() const
{
	return m_baseColor;
}


void Material::setAmbience(float ambientStr)
{
	m_ambientStrength = ambientStr;
}

void Material::setDiffuse(float diffuseStr)
{
	m_diffuseStrength = diffuseStr;
}

void Material::setSpecular(float specularStr)
{
	m_specularStrength = specularStr;
}

void Material::setShininess(float shininess)
{
	m_shininess = shininess;
}

float Material::getDiffuse()
{
	return m_diffuseStrength;
}

float Material::getShininess() const
{
	return m_shininess;
}

float Material::getSpecular() const
{
	return m_specularStrength;
}

float Material::getAmbience() const
{
	return m_ambientStrength;
}

