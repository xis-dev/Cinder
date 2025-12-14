#pragma once

#include <filesystem>
#include <istream>
#include <fstream>
#include <iostream>
#include <string>

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
			auto filePath = base / std::filesystem::path(relativePath);
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
		std::string filePath{};
		filePath = (directory / std::filesystem::path(relativePath)).string();
		if (std::filesystem::exists(directory / std::filesystem::path(relativePath)))
		{
			return filePath;
		}

		std::cout << "FILEMANAGER:: Failed to locate " << relativePath << " at " << directory << '\n';
		return relativePath;
	}

	static std::string getAbsolutePath(const std::string& relativePath)
	{
		std::string absolute = getPath(relativePath);
		auto absolutePath = std::filesystem::absolute(absolute);

		if (std::filesystem::exists(absolutePath))
		{
			return absolutePath.string();
		}

		std::cout << "FILEMANAGER:: Cannot locate absolute path for: " << absolute << '\n';
		return absolute;
	}

	static std::string getAbsolutePath(const std::string& relativePath, const std::string& directory)
	{
		std::string absolute = getPath(relativePath, directory);
		auto absolutePath = std::filesystem::absolute(absolute);

		if (std::filesystem::exists(absolute))
		{
			return absolutePath.string();
		}

		std::cout << "FILEMANAGER:: Cannot locate absolute path at: " << absolute << '\n';
		return absolute;
	}

	static bool fileExists(const std::string& path)
	{
		std::ifstream file(path);

		return file.good();
	}
		
};