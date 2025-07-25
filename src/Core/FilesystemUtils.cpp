#include "FilesystemUtils.h"
#include <SDL.h>

namespace
{
    std::string m_rootDirectory;
}

void Filesystem::setRootDirectory(const std::string &directory_)
{
    m_rootDirectory = directory_ + "/";
    std::replace(m_rootDirectory.begin(), m_rootDirectory.end(), '\\', '/');
}

std::string Filesystem::getRootDirectory()
{
    if (m_rootDirectory.empty())
    {
        auto pathptr = SDL_GetBasePath();
        if (!pathptr)
            throw std::runtime_error("Failed to identify base path: " + std::string(SDL_GetError()));

        std::string path = pathptr;
        SDL_free(pathptr);
        std::filesystem::path ppath(path);

        for (; ppath.filename() != "build" && !ppath.empty(); ppath = ppath.parent_path());

        if (ppath.empty())
            throw std::runtime_error("Failed to find \"build/\" directory");

        m_rootDirectory = ppath.parent_path().string() + "/";
        std::replace(m_rootDirectory.begin(), m_rootDirectory.end(), '\\', '/');
    }

    return m_rootDirectory;
}

void Filesystem::ensureDirectoryRelative(const std::string &directory_)
{
    std::filesystem::create_directory(getRootDirectory() + directory_);
}

std::string Filesystem::getRelativePath(const std::filesystem::path &basePath_, const std::filesystem::path &fullPath_)
{
    auto p = std::filesystem::relative(fullPath_, basePath_).string();
    std::replace(p.begin(), p.end(), '\\', '/');
    return p;
}

std::string Filesystem::removeExtention(const std::string &filePath_)
{
    size_t lastindex = filePath_.find_last_of("."); 
    std::string rawName = filePath_.substr(0, lastindex); 

    return rawName;
}
