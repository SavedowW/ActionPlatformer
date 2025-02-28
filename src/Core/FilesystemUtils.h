#ifndef FILESYSTEM_UTILS_H_
#define FILESYSTEM_UTILS_H_
#include <filesystem>
#include <string>

// All paths returned by functions here end with '/'
namespace Filesystem
{
    void setRootDirectory(const std::string &directory_);
    std::string getRootDirectory();

    /*
        Takes relative path ("Resources")
        Expects parent directory to exist
            Example: "Resporces/Sprites" will throw an exception if "Resources" does not exist
    */
    void ensureDirectoryRelative(const std::string &directory_);

    std::string getRelativePath(const std::filesystem::path &basePath_, const std::filesystem::path &fullPath_);
    std::string removeExtention(const std::string &filePath_);
}

#endif
