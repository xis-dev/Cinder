#include "includes/Resources/Model.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include <iostream>
#include <utility>

#include "Material.h"


Model::Model(ModelSet&& set)
{
	m_meshes.push_back(std::move(set));
}

Model & Model::operator=(Model &&model) noexcept {

	if (this != &model) {
		for (int i = 0; i < m_meshes.size(); ++i) {
			m_meshes[i].mesh = std::exchange(model.m_meshes[i].mesh, Mesh{});
			m_meshes[i].mat = std::exchange(model.m_meshes[i].mat, Handle<Material>::InvalidHandle);
		}
	}

	return *this;
}

Model::Model(Model &&model) noexcept {

	for (int i = 0; i < m_meshes.size(); ++i) {
		m_meshes[i].mesh = std::exchange(model.m_meshes[i].mesh, Mesh{});
		m_meshes[i].mat = std::exchange(model.m_meshes[i].mat, Handle<Material>::InvalidHandle);
	}
}


void Model::add(ModelSet&& set)
{
	m_meshes.push_back(std::move(set));
}

const std::vector<ModelSet>& Model::getMeshes() const
{
	return m_meshes;
}

