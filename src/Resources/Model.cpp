#include "includes/Resources/Model.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include <iostream>
#include "Material.h"


Model::Model(ModelSet&& set)
{
	m_meshes.push_back(std::move(set));
}


void Model::add(ModelSet&& set)
{
	m_meshes.push_back(std::move(set));
}

const std::vector<ModelSet>& Model::getMeshes() const
{
	return m_meshes;
}

