#include "TextureManager.h"

TextureManager::TextureManager(Renderer* renderer_, const std::string &rootPath_) :
	m_renderer(renderer_),
	m_rootPath(rootPath_ + "/")
{
	//Init pointers to textures
	for (int i = 0; i < int(TEXTURES::NONE); ++i)
	{
		m_textures[TEXTURES(i)] = std::weak_ptr<Texture>();
	}
}

std::shared_ptr<Texture> TextureManager::getTexture(TEXTURES tex_)
{
	if (m_textures[tex_].expired())
	{
		//Logger::print("Texture does not exist, creating new\n");
		std::shared_ptr<Texture> reqElem(new Texture(m_renderer->loadTexture((m_rootPath + m_files[(int)tex_]).c_str())));
		m_textures[tex_] = reqElem;
		return reqElem;
	}
	else
	{
		//Logger::print("Texture already exist\n");
		return m_textures[tex_].lock();
	}
}