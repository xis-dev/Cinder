#pragma once

#include <Objects/Headers/Shader.h>
#include <Objects/Headers/Texture.h>
#include <Math/Vec3.h>


class Material
{
public:
	Material(const std::shared_ptr<Shader>& shader) : m_shader(shader)
	{
	}
	Material(const std::shared_ptr<Shader>& shader, const std::shared_ptr<Texture>& texture) : m_shader(shader)
	{
		textureExists = true;
		m_textures.push_back(texture);
	}

	std::string m_tag{};
	void old_material_use(const glm::mat4& projection, const glm::mat4& view, const glm::mat4& model, glm::vec3 viewPosition);
	void addTexture(const std::shared_ptr<Texture>& texture);
	
	void sendShaderInput(const std::string &name, float value);
	void sendShaderInput(const std::string &name, int value);
	void sendShaderInput(const std::string &name, Vec3f value);
	void sendShaderInput(const std::string &name, glm::vec3 value);

	
	void useShader();
	const std::shared_ptr<Shader>& getShader();
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
	bool textureExists{};

public:

	std::shared_ptr<Shader> m_shader{};
	std::vector<std::shared_ptr<Texture>> m_textures{};


};
