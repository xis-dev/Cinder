#pragma once

#include <filesystem>
#include <istream>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class FileManager
{
public:
	static std::string getPath(const std::string& relativePath)
	{
		if (std::filesystem::exists(std::filesystem::path(relativePath)))
		{
			return relativePath;
		}

		std::vector<std::string> basePaths{
			".",
			"..",
			"../..",
			"../../..",
			"../../../..",
			"../../../../..",
			"../../../../../..",
			"../../../../../../../..",
			"../../../../../../../../..",
			"../../../../../../../../../..",
			"../../../../../../../../../../..",


		};

		for (auto& base: basePaths)
		{
			std::filesystem::path filePath = std::filesystem::path(base) / relativePath;
			if (std::filesystem::exists(filePath))
			{
				return filePath.string();
			}
		}


		std::cout << "FILEMANAGER:: Failed to locate file at: " << relativePath << '\n';
		std::cout << "FILEMANAGER:: Current path: " << std::filesystem::current_path() << '\n';

		return relativePath;
	}

	static std::string getPath(const std::string& relativePath, const std::string& directory)
	{
		std::filesystem::path filePath{};
		filePath = (std::filesystem::path(directory) / relativePath);
		if (std::filesystem::exists(filePath))
		{
			return filePath.string();
		}

		std::cout << "FILEMANAGER:: Failed to locate " << relativePath << " at " << directory << '\n';
		return relativePath;
	}

	static std::string getCanonicalPath(const std::string& relativePath)
	{
		std::string absolute = getPath(relativePath);
		std::filesystem::path absolutePath;
		try
		{
		absolutePath = std::filesystem::canonical(absolute);
		}
		catch (std::exception e)
		{
			std::cerr << "Exception: " << e.what() << std::endl;
		}

		if (std::filesystem::exists(absolutePath))
		{
			return absolutePath.string();
		}

		std::cout << "FILEMANAGER:: Cannot locate absolute path for: " << absolute << '\n';
		return absolute;
	}

	static std::string getCanonicalPath(const std::string& relativePath, const std::string& directory)
	{
		std::filesystem::path resolved = std::filesystem::absolute(std::filesystem::path(directory) / relativePath);

		if (std::filesystem::exists(resolved))
		{
			return resolved.string();
		}

		std::cout << "FILEMANAGER:: Cannot locate absolute path at: " << resolved << '\n';
		return resolved.string();
	}

	static bool fileExists(const std::string& path)
	{
		std::ifstream file(path);

		return file.good();
	}
		
};