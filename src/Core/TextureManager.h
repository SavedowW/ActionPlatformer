#ifndef TEXTURE_MANAGER_H_
#define TEXTURE_MANAGER_H_

#include "Renderer.h"
#include <memory>
#include <map>

enum class TEXTURES
{
	NONE
};

class TextureManager
{
public:
	TextureManager(Renderer* renderer_, const std::string &rootPath_);
	std::shared_ptr<Texture> getTexture(TEXTURES tex);

private:
	std::string m_rootPath;
	Renderer* m_renderer;
	std::map<TEXTURES, std::weak_ptr<Texture>> m_textures;
	const std::string m_files[(int)TEXTURES::NONE] = {
		"Resources/Sprites/Stage1/Background_p1.png",
		"Resources/Sprites/Stage1/Background_p2.png",
		"Resources/Sprites/HUD/healthbar.png",
		"Resources/Sprites/HUD/healthbarBack.png",
		"Resources/Sprites/HUD/healthbarFront.png"
	};
};

#endif