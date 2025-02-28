#ifndef TEXTURE_MANAGER_H_
#define TEXTURE_MANAGER_H_

#include "Renderer.h"
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
	TextureManager(Renderer* renderer_);
	std::shared_ptr<Texture> getTexture(int id_);
	void preload(const std::string &toPreload_);
	void preload(int toPreload_);

	int getTexID(const std::string &texName_) const;

private:
	Renderer* m_renderer;
	std::map<std::string, int> m_ids;
	std::vector<ContainedTextureData> m_textures_;
};

#endif