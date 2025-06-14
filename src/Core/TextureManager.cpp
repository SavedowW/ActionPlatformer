#include "TextureManager.h"
#include "FilesystemUtils.h"
#include <SDL_Image.h>

TextureManager::TextureManager(Renderer &renderer_) :
	m_renderer(renderer_)
{
	auto sprPath = Filesystem::getRootDirectory() + "Resources/Sprites/";
	std::filesystem::path basePath(sprPath);

	std::cout << "=== LISTING FOUND TEXTURES ===\n";
	for (const auto &entry : std::filesystem::recursive_directory_iterator(sprPath))
	{
		std::filesystem::path dirpath = entry.path();
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

std::shared_ptr<Texture> TextureManager::getTexture(int id_)
{
	if (m_textures_[id_].m_preloaded)
	{
		//Logger::print("Texture arr is preloaded\n");
		return m_textures_[id_].m_preloaded;
	}
	else if (m_textures_[id_].m_tex.expired())
	{
		//auto sprPath = m_rootPath + "/Resources/Sprites/";
		auto reqElem = loadTexture((m_textures_[id_].m_path).c_str());
		m_textures_[id_].m_tex = reqElem;
		return reqElem;
	}
	else
	{
		return m_textures_[id_].m_tex.lock();
	}
}

void TextureManager::preload(const std::string &toPreload_)
{
	int id = getTexID(toPreload_);

	preload(id);
}

void TextureManager::preload(int toPreload_)
{
	if (m_textures_[toPreload_].m_preloaded == nullptr)
	{
		m_textures_[toPreload_].m_preloaded = getTexture(toPreload_);
	}
}

int TextureManager::getTexID(const std::string &texName_) const
{
    try
	{
    	return m_ids.at(texName_);
	}
	catch (std::out_of_range exc_)
	{
		throw std::string("Failed to find texture ") + texName_ + " : " + exc_.what();
	}
}

std::shared_ptr<TextureResource> TextureManager::loadTexture(const std::string &path_)
{
    auto *imgSurface = IMG_Load(path_.c_str());

    if (!imgSurface)
    {
        std::cout << "Failed to load texture \"" << path_ << "\"" << std::endl;
        return nullptr;
    }

    
    std::cout << SDL_GetPixelFormatName(imgSurface->format->format) << std::endl;

    Vector2<int> texSize(imgSurface->w, imgSurface->h);
    unsigned int texId = 0;

    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSize.x, texSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, imgSurface->pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);

    dumpErrors();

    SDL_FreeSurface(imgSurface);

    return std::make_shared<TextureResource>(path_, texSize, texId);
}
