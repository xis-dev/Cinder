#include "includes/Resources/Model.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include <iostream>
#include "Material.h"


Model::Model(const ModelSet& set)
{
	m_meshes.push_back(set);
}

void Model::add(const ModelSet& set)
{
	m_meshes.push_back(set);
}

const std::vector<ModelSet>& Model::getMeshes() const
{
	return m_meshes;
}

