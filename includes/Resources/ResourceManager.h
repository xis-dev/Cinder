#pragma once

#include <unordered_map>
#include <memory>
#include <typeindex>

#include "Resources/Resource.h"

#include "Resources/Shader.h"
#include "Resources/Texture.h"
#include "Resources/Material.h"
#include "Resources/Mesh.h"

template <typename T>
requires std::derived_from<T, Resource>
class ResourceManager {
	
	std::unordered_map<std::string, std::unique_ptr<T>> m_resourceMap{};

	std::string m_defaultResourceName{};
	int m_defaultIndex{};
	
public:

	ResourceManager(const std::string& defaultResourceName): m_defaultResourceName(defaultResourceName){}

	T* addResource(const std::string& name, const T& resource)
	{
		auto resourceName = name;
		if (name.empty())
		{
			resourceName = getCurrentDefaultName();
		}
		if (m_resourceMap.contains(name))
		{
			std::cerr << "RESOURCE MANAGER:: A resource with this name already exists. \n";
			return nullptr;
		}
		m_resourceMap[resourceName] = std::make_unique<T>(resource);
		return m_resourceMap[resourceName].get();
	}

	T* addOrReplaceResource(const std::string& name, const T& resource)
	{
		auto resourceName = name;
		if (name.empty())
		{
			resourceName = getCurrentDefaultName();
		}
		if (m_resourceMap.contains(name))
		{
			std::cout << "RESOURCE MANAGER:: A resource with this name already exists. Replacing current resource. \n";
		}
		m_resourceMap[resourceName] = std::make_unique<T>(resource);
		return m_resourceMap[resourceName].get();
	}

	void clear()
	{
		m_resourceMap.clear();
	}

	T* tryGetResource(const std::string& name)
	{

		if (!m_resourceMap.contains(name))
		{
			std::cerr << "RESOURCE MANAGER:: There is no resource with the provided name.\n";
			return nullptr;
		}

		return m_resourceMap[name].get();

	}

	// TODO: 
	std::vector<std::pair<std::string, T*>> getAllResources()
	{
		std::vector<std::pair<std::string, T*>> out;
		out.reserve(m_resourceMap.size());
		for (auto& kv : m_resourceMap) out.emplace_back(kv.first, kv.second.get());
		return out;
	}

	std::vector<std::string> getAllResourceNames()
	{
		std::vector<std::string> names;
		names.reserve(m_resourceMap.size());
		for (auto& kv : m_resourceMap)
		{
			names.emplace_back(kv.first);
		}
		return names;
	}

private:
	std::string getCurrentDefaultName()
	{
		if (!m_resourceMap.contains(m_defaultResourceName) && m_defaultIndex == 0)
		{
			return m_defaultResourceName;
		}

		++m_defaultIndex;
		return m_defaultResourceName + std::to_string(m_defaultIndex);
	}

};

