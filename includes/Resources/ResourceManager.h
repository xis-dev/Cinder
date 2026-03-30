#pragma once



#include "Resources/Resource.h"

#include "Resources/Shader.h"
#include "Resources/Texture.h"
#include "Resources/Material.h"
#include "Resources/Mesh.h"
#include "Resources/Model.h"


#include <unordered_map>
#include <memory>
#include <typeindex>
#include <utility>
#include <vector>

#include "Resources/Handle.h"
template <typename T>
requires std::derived_from<T, Resource>
class ResourceManager {

	std::vector<std::unique_ptr<T>> resources{};
	std::unordered_map<std::string, std::uint32_t> nameToIndex{};


public:

	Handle<T> add(T&& resource, const std::string& name)
	{
		uint32_t id = resources.size();
		resources.push_back(std::make_unique<T>(std::move(resource)));
		nameToIndex[name] = id;
		return { id };
	}

	T* get(Handle<T> handle)
	{
		return resources[handle.id].get();
	}

	T* get(const std::string& name)
	{
		auto& handleID = nameToIndex[name];
		return resources[handleID].get();
	}

	Handle<T> getHandle(const std::string& name)
	{
		return Handle<T>{nameToIndex[name]};
	}

	const std::vector<std::unique_ptr<T>>& getAllResources()
	{
		return resources;
	}

	std::vector<std::pair<std::string, std::uint32_t>> getNames() const
	{
		std::vector<std::pair<std::string, std::uint32_t>> temp{};
		for (auto& pair : nameToIndex)
		{
			temp.push_back({ pair.first, pair.second });
		}
		return temp;
	}

};

