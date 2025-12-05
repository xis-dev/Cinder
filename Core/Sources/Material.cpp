#include "Core/Headers/Material.h"


void Material::old_material_use(const glm::mat4 &projection, const glm::mat4 &view, const glm::mat4 &model,
	const glm::vec3 viewPosition)
{
	m_shader->use();

	if (textureExists) {
		int diffuseNr = 0;
		int specularNr = 0;
		for (int i = 0; i < m_textures.size(); ++i) {
			std::string uniformStr;
			auto& currentTex = m_textures[i];

			switch (currentTex->type) {
				case Texture::Diffuse:
					uniformStr = "t_Diffuse[" + std::to_string(diffuseNr) + "]";
					m_shader->setUniformi(uniformStr.c_str(), i);  // Use 'i' not '0'
					++diffuseNr;
					break;

				case Texture::Specular:
					uniformStr = "t_Specular[" + std::to_string(specularNr) + "]";
					m_shader->setUniformi(uniformStr.c_str(), i);  // Use 'i' not '0'
					++specularNr;
					break;
			}

			glActiveTexture(GL_TEXTURE0 + i);
			currentTex->bind();
		}

		// Set correct counts
		m_shader->setUniformi("u_DiffuseMapCount", diffuseNr);
		m_shader->setUniformi("u_SpecularMapCount", specularNr);

		glActiveTexture(GL_TEXTURE0);
	}


	std::string materialBaseName = "u_Material.";
	m_shader->setUniformVec3((materialBaseName + "albedo").c_str(), m_baseColor);
	m_shader->setUniformf((materialBaseName + "ambient").c_str(), m_ambientStrength);
	m_shader->setUniformf((materialBaseName + "diffuse").c_str(), m_diffuseStrength);
	m_shader->setUniformf((materialBaseName + "specular").c_str(), m_specularStrength);
	m_shader->setUniformf((materialBaseName + "shininess").c_str(), m_shininess);

	m_shader->setUniformMat4("u_ModelMatrix", model);
	m_shader->setUniformMat4("u_MVPMatrix", projection * view * model);

if (textureExists)
{
    int diffuseNr = 0;
    int specularNr = 0;
    int textureUnit = 0; 

    for (int i = 0; i < (int)m_textures.size(); ++i)
    {
        auto& currentTex = m_textures[i];
        std::string uniformStr;

        if (currentTex->type == Texture::Diffuse)
        {
            uniformStr = "t_Diffuse[" + std::to_string(diffuseNr) + "]";
            ++diffuseNr;
        }
        else if (currentTex->type == Texture::Specular)
        {
            uniformStr = "t_Specular[" + std::to_string(specularNr) + "]";
            ++specularNr;
        }
        else
        {
            uniformStr = "t_Diffuse[" + std::to_string(diffuseNr) + "]";
            ++diffuseNr;
        }

        m_shader->setUniformi(uniformStr.c_str(), textureUnit);

        glActiveTexture(GL_TEXTURE0 + textureUnit);
        currentTex->bind();  
        ++textureUnit;
    }

    m_shader->setUniformi("u_DiffuseMapCount", diffuseNr);
    m_shader->setUniformi("u_SpecularMapCount", specularNr);

    glActiveTexture(GL_TEXTURE0);
}

	m_shader->setUniformVec3("u_CameraPosition", viewPosition);
	
}

void Material::use()
{
}

void Material::addTexture(const std::shared_ptr<Texture>& texture)
{
	textureExists = true;
	m_textures.push_back(texture);
	std::cout << "Added new texture to material: " << m_tag << "\n";
}


void Material::sendShaderInput(const std::string& name, float value)
{
	m_shader->setUniformf(name.c_str(), value);
}

void Material::sendShaderInput(const std::string& name, int value)
{
	m_shader->setUniformi(name.c_str(), value);
}

void Material::sendShaderInput(const std::string& name, Vec3f value)
{
	m_shader->setUniformVec3(name.c_str(), glm::vec3(value.x, value.y, value.z));
}

void Material::sendShaderInput(const std::string& name, glm::vec3 value)
{
	m_shader->setUniformVec3(name.c_str(), glm::vec3(value.x, value.y, value.z));
}


void Material::useShader()
{
	if (!m_shader)
	{
		std::cerr << "MATERIAL:: NULL SHADER FOUND. \n";
	}
	m_shader->use();
}

const std::shared_ptr<Shader>& Material::getShader()
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

