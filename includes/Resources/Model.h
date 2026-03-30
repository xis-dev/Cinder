#pragma once

#include "Resources/Resource.h"
#include "Resources/Mesh.h"

#include <unordered_map>
#include <memory>
#include <string>

#include "Handle.h"

struct aiNode;
struct aiScene;
struct aiMesh;
class Shader;
class Material;

struct ModelSet {
	Mesh mesh;
	Handle<Material> mat;
};
class Model: public Resource {
public:
	Model() = default;
	Model(const ModelSet& set);

	// Use vector of pairs instead of unordered_map
	std::vector<ModelSet> m_meshes;

	void add(const ModelSet& set);

	const std::vector<ModelSet>& getMeshes() const;

	void destroy() override {}
};