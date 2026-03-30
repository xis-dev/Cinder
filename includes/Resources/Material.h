#pragma once

#include <vector>
#include "Math/Vec3.h"
#include "Resources/Resource.h"

#include "Resources/Handle.h"

class Shader;
class Texture;

class Material: public Resource
{
public:
	Material(Handle<Shader> shader) : m_shader(shader)
	{
	}
	Material(Handle<Shader> shader, Handle<Texture> texture) : m_shader(shader)
	{
		m_textures.push_back(texture);
	}
	void addTexture(Handle<Texture> texture);
	
	Handle<Shader> getShader() const;
	std::vector<Handle<Texture>> getTextures();
	void setColor(Vec3f color);
	void setColor(float r, float g, float b);
	void setAmbience(float ambientStr);
	void setDiffuse(float diffuseStr);
	void setSpecular(float specularStr);
	void setShininess(float shininess);
	float getDiffuse();
	float getShininess() const;
	float getSpecular() const;
	float getAmbience() const;
	Vec3f getColor() const;



private:
	Vec3f m_baseColor{1.0f};
	float m_ambientStrength{1.0f};
	float m_diffuseStrength{ 1.0f };
	float m_specularStrength{ 0.5f }; // arbitrary default for normal
	float m_shininess{ 32.0f };

public:

	Handle<Shader> m_shader{};
	std::vector<Handle<Texture>> m_textures{};

	virtual void destroy() override {};


};
