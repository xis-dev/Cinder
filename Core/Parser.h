#pragma once

#include <Core/Headers/Mesh.h>
#include <Core/FileManager.h>

class Parser
{
	static std::shared_ptr<Mesh> loadOBJMesh(const std::string& path)
	{
		std::ifstream fileStream;
		if (FileManager::fileExists(path))
		{
			fileStream = std::ifstream(path);
		}
		else
		{
			std::cout << "PARSER::OBJ:: failed to find " << path << std::endl;
			return std::make_shared<Mesh>();
		}

		std::string typeSymbol;
		while (fileStream >> typeSymbol)
		{
			if (typeSymbol == "v")
			{
				
			}
			if (typeSymbol == "vn") {}
			if (typeSymbol == "vt") {}
			if (typeSymbol == "f")
			{
				
			}

		}

		return std::make_shared<Mesh>();
	}
};
