#ifndef TEXTURE_H_
#define TEXTURE_H_

#include <SDL.h>
#include <SDL_Image.h>
#include <memory>

struct Texture
{
	//Makes sure texture actually exist
	Texture(SDL_Texture* tex_);

	Texture();

	Texture(Texture &tex_) = delete;
	Texture &operator=(Texture &rhs_) = delete;

	Texture(Texture &&tex_);

	Texture &operator=(Texture &&rhs_);

	void setTexture(SDL_Texture *tex_);

	SDL_Texture* getSprite();

	//Properly removes texture
	virtual ~Texture();

	int m_w, m_h;

private:
	SDL_Texture* m_tex;
};

using Texture_t = std::shared_ptr<Texture>;

#endif