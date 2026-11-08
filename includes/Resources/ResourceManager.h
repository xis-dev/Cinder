#pragma once


#include <format>

#include "Resources/Resource.h"
#include <unordered_map>
#include <memory>
#include <utility>
#include <vector>
#include <iostream>

#include "Resources/Handle.h"
template <typename T>
requires std::derived_from<T, Resource>
class ResourceManager {
	std::unordered_map<Handle<T>, std::unique_ptr<T>> resources{};
	std::unordered_map<std::string, Handle<T>> namedResourceHandles{};

	static Handle<T> GetNextHandle() {
		static Handle<T> handle = {0};
		++handle.id;
		return handle;
	}
public:

	Handle<T> add(T&& resource) {
		Handle<T> handle = GetNextHandle();

		std::string finalName;
		resources[handle] = std::make_unique<T>(std::move(resource));

		return handle;
	}

	// Named resources
	Handle<T> addNamed(T&& resource, const std::string& name)
	{
		if (namedResourceHandles.contains(name)) {
			// TODO: Warning with logging instead
			std::cerr << std::format("RESOURCE_MANAGER: A resource with this name '{}' already exists. The named resource will be replaced with the most recent addition\n", name);
		}

		Handle<T> handle = add(std::move(resource));
		namedResourceHandles[name] = handle;
		return handle;
	}

	T* get(Handle<T> handle)
	{
		auto resourceIt = resources.find(handle);
		if (resourceIt != resources.end()) {
			return resourceIt->second.get();
		}

		return nullptr;
	}

	T* getNamed(const std::string& name)
	{
		auto handleIt = namedResourceHandles.find(name);
		if (handleIt != namedResourceHandles.end())
		{
			auto resourceIt = resources.find(handleIt->second);
			if (resourceIt != resources.end()) {
				return resourceIt->second.get();
			}
		}

		return nullptr;
	}

	Handle<T> getNamedHandle(const std::string& name)
	{
		auto handleIt = namedResourceHandles.find(name);
		if (handleIt != namedResourceHandles.end()) {
			return handleIt->second;
		}

		return Handle<T>::InvalidHandle;
	}

	const std::unordered_map<Handle<T>, std::unique_ptr<T>>& getAllResources() const
	{
		return resources;
	}

	const std::unordered_map<std::string, Handle<T>>& getNamedResources() const
	{
		return namedResourceHandles;
	}


};


