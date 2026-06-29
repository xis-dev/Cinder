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
	std::unordered_map<std::string, std::uint32_t> nameToHandleID{};
	std::unordered_map<std::string, std::uint32_t> nameToCount{};

public:

	Handle<T> add(T&& resource, const std::string& name)
	{
		// TODO: Add character rejection(?,.,etc)
		uint32_t id = resources.size();
		std::string finalName;

		auto lastUnderscore = name.find_last_of("_");
		bool noUnderscore = lastUnderscore == std::string::npos; // Could we find an underscore in the string
		std::string actualName = noUnderscore ? name : name.substr(0, lastUnderscore); // Name devoid of the last found underscore
		// Everything after the last found underscore
		std::string postUnderscoreString = noUnderscore ? "" : name.substr((lastUnderscore + 1), name.length() - (lastUnderscore+1));

		// If there is an underscore and the name's length is the same as it +1(the underscore ends the string, treat it as if it were a fresh string and do not truncate the _)
		bool postStrIsNumber = noUnderscore ? false : (lastUnderscore + 1) != name.length();
		for (size_t i = 0; i < postUnderscoreString.length(); ++i)
		{
			if (!isdigit(postUnderscoreString[i]))
			{
				postStrIsNumber = false;
				break;
			}
		}

		// Was the post string a number and the underscore not the last element? if not, use the given name
		std::string nameToLookFor = postStrIsNumber ? actualName : name;
		std::unordered_map<std::string, std::uint32_t>::iterator nameAndCount = nameToCount.find(nameToLookFor);

		if (nameAndCount == nameToCount.end()) // Name was not found and we're at end of the map
		{
			// Add this name to the map and use just the name we were looking for
			finalName = nameToLookFor;
			nameToCount.insert({finalName, 1});
		}
		else
		{
			auto count = nameAndCount->second;
			finalName = nameToLookFor + "_" + std::to_string(count); // Get name stripped of last underscore if one existed then get the current count of that name in the map and append
			nameToCount[nameToLookFor] = count + 1;
		}

		nameToHandleID[finalName] = id;
		resources.push_back(std::make_unique<T>(std::move(resource)));
		return { id };
	}

	T* get(Handle<T> handle)
	{
		return resources[handle.id].get();
	}

	T* get(const std::string& name)
	{
		auto& handleID = nameToHandleID[name];
		return resources[handleID].get();
	}

	Handle<T> getHandle(const std::string& name)
	{
		return Handle<T>{nameToHandleID[name]};
	}

	const std::vector<std::unique_ptr<T>>& getAllResources()
	{
		return resources;
	}

	std::vector<std::pair<std::string, std::uint32_t>> getNames() const
	{
		std::vector<std::pair<std::string, std::uint32_t>> temp{};
		for (auto& pair : nameToHandleID)
		{
			temp.push_back({ pair.first, pair.second });
		}
		return temp;
	}



};

