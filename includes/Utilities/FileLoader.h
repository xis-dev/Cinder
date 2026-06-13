#pragma once

#include "ModelLoader.h"

#include <filesystem>
class FileLoader
{

    enum LoadableExtensions
    {
        NONE,
        OBJ, FBX, GLTF,
        PNG, JPG
    };

    static const std::string loadableExtensions[];
    std::unique_ptr<ModelLoader> m_modelLoader{};

private:
    std::string getExtensionString(LoadableExtensions extensionType);
    LoadableExtensions getExtensionType(const std::string& extension);
public:
    bool loadFile(const char* file);

    bool getPath(const std::string& relativePath, std::filesystem::path& outPath);
    bool getCanonicalPath(const std::string& relativePath, std::filesystem::path& outPath);
    bool getWeakPath(const std::string& relativePath, std::filesystem::path& outPath);

};