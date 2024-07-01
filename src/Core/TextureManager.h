#ifndef TEXTURE_MANAGER_H_
#define TEXTURE_MANAGER_H_

#include "Renderer.h"
#include <memory>
#include <map>

enum class TEXTURES
{
	NONE
};

struct Texture
{
	//Makes sure texture actually exist
	Texture(SDL_Texture* tex_)
	{
		if (tex_ == NULL)
			throw "Texture does not exist!\n";

		m_tex = tex_;

		SDL_QueryTexture(m_tex, NULL, NULL, &m_w, &m_h);
	}

	//The texture and required info
	SDL_Texture* getSprite()
    {
	    return m_tex;
    }
	int m_w, m_h;

	//Properly removes texture
	virtual ~Texture()
	{
		//Logger::print("Release single texture\n");
		SDL_DestroyTexture(m_tex);
	}

private:
	SDL_Texture* m_tex;
};

using Texture_t = std::shared_ptr<Texture>;

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