#pragma once

#include "Resources/Texture.h"

#include <typeindex>
#include <unordered_map>

#include "Component.h"
#include "CinUtility.h"

class Entity;

class IconRegistry
{
	inline static std::unordered_map<ComponentTypeID, Texture*> m_iconMap{};

public:
	template <CinUtility::DerivedConcept<Component> T>
	static void registerType(Texture* iconImage)
	{
		m_iconMap[ComponentUtil::getComponentID<T>()] = iconImage;
	}

	template <CinUtility::DerivedConcept<Component> T>
	static Texture* tryGetIcon()
	{
		const auto& iterator = m_iconMap.find(ComponentUtil::getComponentID<T>());
		if (iterator != m_iconMap.end())
		{
			return iterator->second;
		}
		return nullptr;
	}

	static Texture* tryGetIcon(ComponentTypeID componentID)
	{
		const auto& iterator = m_iconMap.find(componentID);
		if (iterator != m_iconMap.end())
		{
			return iterator->second;
		}
		return nullptr;
	}

};


