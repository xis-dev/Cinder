#include "Resources/Material.h"
#include <Resources/Shader.h>
#include <Resources/Texture.h>




void Material::addTexture(Handle<Texture> texture)
{

m_textures.push_back(texture);
}


Handle<Shader> Material::getShader() const
{
	return m_shader;
}

std::vector<Handle<Texture>> Material::getTextures()
{
	return m_textures;
}

void Material::setColor(glm::vec3 color)
{
	m_baseColor = color;
}

void Material::setColor(float r, float g, float b)
{
	m_baseColor = glm::vec3(r,g,b);
}

glm::vec3 Material::getColor() const
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

