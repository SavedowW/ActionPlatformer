#include "TextureManager.h"
#include "FilesystemUtils.h"
#include "Logger.hpp"
#include "SDLWrappers.h"
#include <SDL3_image/SDL_image.h>

TextureManager::TextureManager()
{
    Filesystem::ensureDirectoryRelative("Resources/Sprites");
    const std::filesystem::path basePath(Filesystem::getRootDirectory() + "Resources/Sprites/");

    std::cout << "=== LISTING FOUND TEXTURES ===\n";
    for (const auto &entry : std::filesystem::recursive_directory_iterator(basePath))
    {
        const std::filesystem::path &dirpath = entry.path();
        if (entry.is_regular_file() && dirpath.extension() == ".png")
        {
            auto path = Filesystem::getRelativePath(basePath, dirpath);
            auto noExtension = Filesystem::removeExtention(path);
            std::cout << noExtension << std::endl;

            ContainedTextureData ctd;
            ctd.m_path = dirpath.string();

            m_textures_.push_back(ctd);
            m_ids[noExtension] = m_textures_.size() - 1;
        }
    }
    std::cout << "=== LISTING ENDS HERE ===\n";
}

std::shared_ptr<Texture> TextureManager::getTexture(ResID id_)
{
    if (m_textures_[id_].m_preloaded)
    {
        //Logger::print("Texture arr is preloaded\n");
        return m_textures_[id_].m_preloaded;
    }

    if (m_textures_[id_].m_tex.expired())
    {
        //auto sprPath = m_rootPath + "/Resources/Sprites/";
        auto reqElem = loadTexture(m_textures_[id_].m_path);
        m_textures_[id_].m_tex = reqElem;
        return reqElem;
    }
    
    return m_textures_[id_].m_tex.lock();
}

void TextureManager::preload(const std::string &toPreload_)
{
    preload(getTexID(toPreload_));
}

void TextureManager::preload(ResID id_)
{
    if (m_textures_[id_].m_preloaded == nullptr)
    {
        m_textures_[id_].m_preloaded = getTexture(id_);
    }
}

ResID TextureManager::getTexID(const std::string &texName_) const
{
    try
    {
        return m_ids.at(texName_);
    }
    catch (const std::out_of_range &exc_)
    {
        throw std::runtime_error("Failed to find texture " + texName_ + " : " + exc_.what());
    }
}

std::shared_ptr<Texture> TextureManager::loadTexture(const std::string &path_)
{
    SurfaceWrapper imgSurface{IMG_Load(path_.c_str())};
    
    LOG_TRACE("Loaded {}: {}", path_, SDL_GetPixelFormatName(imgSurface.get().format));

    return std::make_shared<Texture>(Texture::Config{imgSurface});
}
