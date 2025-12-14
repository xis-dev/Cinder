#pragma once

#include "Resources/Texture.h"

#include <typeindex>
#include <unordered_map>

class Entity;

class IconRegistry
{

	inline static std::unordered_map<std::type_index, Texture*> m_iconMap{};

public:
	template <typename T>
	static void registerType(Texture* iconImage)
	{
		if constexpr (std::is_base_of_v<Entity, T>)
		{
			m_iconMap[typeid(T)] = iconImage;
		}
	}


	template <typename T>
	static Texture* tryGetIcon()
	{
		if constexpr (std::is_base_of_v<Entity, T>)
		{
			auto iterator = m_iconMap.find(typeid(T));
			if (iterator != m_iconMap.end())
			{
				return iterator->second;
			}
		}		
		return nullptr;
	}

};


