#include "FilesystemUtils.h"
#include <SDL3/SDL.h>

#include <algorithm>
#include <filesystem>

namespace
{
    // TODO: find a way to avoid iteration on real release build
    std::string generateRootDirectory()
    {
        const auto *pathptr = SDL_GetBasePath();
        if (!pathptr)
            throw std::runtime_error("Failed to identify base path: " + std::string(SDL_GetError()));

        std::string path = pathptr;
        std::filesystem::path ppath(path);

        for (; ppath.filename() != "build" && !ppath.empty(); ppath = ppath.parent_path());

        if (ppath.empty())
            throw std::runtime_error("Failed to find \"build/\" directory");

        auto rootDir = ppath.parent_path().string() + "/";
        std::ranges::replace(rootDir, '\\', '/');

        return rootDir;
    }

    const std::string m_rootDirectory = generateRootDirectory();
}

std::string Filesystem::getRootDirectory()
{
    return m_rootDirectory;
}

void Filesystem::ensureDirectoryRelative(const std::string &directory_)
{
    std::filesystem::create_directory(getRootDirectory() + directory_);
}

std::string Filesystem::getRelativePath(const std::filesystem::path &basePath_, const std::filesystem::path &fullPath_)
{
    auto p = std::filesystem::relative(fullPath_, basePath_).string();
    std::ranges::replace(p, '\\', '/');
    return p;
}

std::string Filesystem::removeExtention(const std::string &filePath_)
{
    size_t lastindex = filePath_.find_last_of('.'); 
    std::string rawName = filePath_.substr(0, lastindex); 

    return rawName;
}
