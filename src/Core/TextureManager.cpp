#include "TextureManager.h"
#include "FilesystemUtils.h"

TextureManager::TextureManager(Renderer* renderer_) :
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
		std::shared_ptr<Texture> reqElem(new Texture(m_renderer->loadTexture((m_textures_[id_].m_path).c_str())));
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
		m_textures_[toPreload_].m_preloaded = std::shared_ptr<Texture>(getTexture(toPreload_));
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
