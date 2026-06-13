#include "FileLoader.h"




const std::string FileLoader::loadableExtensions[]
{
    ".fbx", ".obj", ".gltf",
    ".jpg", ".png"
};

std::string FileLoader::getExtensionString(LoadableExtensions extensionType)
{
    switch (extensionType)
    {
        case OBJ:{return ".obj";}
        case FBX:{return ".fbx";}
        case GLTF:{return ".gltf";}
        case JPG:{return ".jpg";}
        case PNG:{return ".png";}
        case NONE:
        default:
            return "";
    }
}

FileLoader::LoadableExtensions FileLoader::getExtensionType(const std::string &extension)
{
    if (extension == ".obj") return OBJ;
    if (extension == ".fbx") return FBX;
    if (extension == ".gltf") return GLTF;
    if (extension == ".jpg") return JPG;
    if (extension == ".png") return PNG;
    return NONE;
}

bool FileLoader::loadFile(const char *file)
{
    std::filesystem::path filePath;
    if (getCanonicalPath(file, filePath))
    {
        auto extension = filePath.extension();
        bool fileLoaded{};
        if (!extension.empty())
        {
            LoadableExtensions extType = getExtensionType(extension.string());
            switch (extType)
            {
                case OBJ:
                case FBX:
                case GLTF:
                {
                    std::filesystem::path modelPath{};
                    if (!getPath("assets/Models", modelPath))
                    {
                        std::cout << "Could not find models folder.\n";
                        return false;
                    }
                    std::filesystem::path newFile = modelPath / filePath.filename();
                    try
                    {
                        if (std::filesystem::copy_file(filePath, newFile))
                        {
                            std::cout << "Successfully loaded file: " << filePath << " into assets at: " << newFile << ".\n";
                            return true;
                        }
                        return false;
                    }
                    catch (std::filesystem::filesystem_error& e)
                    {
                        std::cout << "Failed to load file: " << filePath << " into assets at: " << newFile << ".\n";
                        std::cout << "ERROR:: " << e.what() << ".\n";
                    }
                    return true;
                }
                case JPG:
                case PNG:
                {
                    std::cout << "Will load into textures folder: " << getExtensionString(extType) << std::endl;
                    return true;
                }
                case NONE:
                default:
                std::cout << "Could not load file with extension: " << extension << std::endl;
                    return false;
            }
        }
        return false;
    }
    return false;
}

bool FileLoader::getPath(const std::string &relativePath, std::filesystem::path& outPath)
{
    if (std::filesystem::exists(std::filesystem::path(relativePath)))
    {
        outPath = relativePath;
        return true;
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
            outPath = filePath;
            return true;
        }
    }


    std::cout << "FILEMANAGER:: Failed to locate file at: " << relativePath << ", File may be nested too deeply.\n";
    std::cout << "FILEMANAGER:: Current path: " << std::filesystem::current_path() << '\n';

    outPath = std::filesystem::path(relativePath);
    return false;
}


bool FileLoader::getCanonicalPath(const std::string &relativePath, std::filesystem::path &outPath)
{
    if (getPath(relativePath, outPath))
    {
        outPath = std::filesystem::canonical(outPath);
        return true;
    }
    return false;
}

bool FileLoader::getWeakPath(const std::string &relativePath, std::filesystem::path &outPath)
{
    return false;
}

