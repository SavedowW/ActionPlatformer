#pragma once
#include "Renderer.h"
#include "Texture.h"
#include <memory>
#include <map>

struct ContainedTextureData
{
	std::string m_path;
	std::weak_ptr<Texture> m_tex;
	std::shared_ptr<Texture> m_preloaded;
};

class TextureManager
{
public:
	TextureManager();
	std::shared_ptr<Texture> getTexture(ResID id_);
	void preload(const std::string &toPreload_);
	void preload(ResID id_);

	ResID getTexID(const std::string &texName_) const;
    std::shared_ptr<Texture> loadTexture(const std::string &path_);

private:
	std::map<std::string, ResID> m_ids;
	std::vector<ContainedTextureData> m_textures_;
};
