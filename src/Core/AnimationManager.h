#ifndef ANIMATION_MANAGER_H_
#define ANIMATION_MANAGER_H_
#include "TextureManager.h"
#include <fstream>
#include <filesystem>
#include "EngineAnimation.h"

//Texture array structure
struct TextureArr
{
	TextureArr(SDL_Texture** tex_, SDL_Texture** whiteTex_, SDL_Texture** borderTex_, int amount_, int totalDuration_, const std::vector<int> &framesData_, int w_, int h_, const Vector2<float> &origin_) :
		m_tex(tex_), m_whiteTex(whiteTex_), m_borderTex(borderTex_), m_amount(amount_), m_totalDuration(totalDuration_), m_framesData(framesData_), m_w(w_), m_h(h_), m_origin(origin_)
	{
	}

	SDL_Texture* operator[](const int rhs)
	{
		return m_tex[m_framesData[rhs]];
	}

	SDL_Texture *getWhite(const int rhs)
	{
		return m_whiteTex[m_framesData[rhs]];
	}

	SDL_Texture *getBorder(const int rhs)
	{
		return m_borderTex[m_framesData[rhs]];
	}

	//Texture array and required info
	SDL_Texture** m_tex;
	SDL_Texture** m_whiteTex;
	SDL_Texture** m_borderTex;
	int m_amount;
	int m_w, m_h;
	int m_totalDuration;
	Vector2<float> m_origin;
	std::vector<int> m_framesData;

	//Properly removes texture
	virtual ~TextureArr();
};

struct ContainedAnimationData
{
	std::string m_path;
	std::weak_ptr<TextureArr> m_texArr;
	std::shared_ptr<TextureArr> m_preloaded;
};

/**************************
*
*	Animation manager holds all texture arrays
*	and makes sure there is only one version of each
*	texture array exist and only if required
*
**************************/
class AnimationManager
{
public:
	AnimationManager(Renderer* renderer_, const std::string &rootPath_);
	std::shared_ptr<TextureArr> getTextureArr(int id_);
	void preload(const std::string &toPreload_);
	void preload(int toPreload_);

	int getAnimID(const std::string &animName_) const;

private:
	std::string m_rootPath;
	Renderer* m_renderer;

	std::map<std::string, int> m_ids;
	std::vector<ContainedAnimationData> m_textureArrs;
};

enum class LOOPMETHOD
{
	NOLOOP,
	JUMP_LOOP,
	SWITCH_DIR_LOOP
};

/**************************
*
*	Animation class
*	Call update each frame
*	Direction = 1 - direct order
*	Direction = -1 - reverse order
*
**************************/
class Animation
{
public:
	Animation(AnimationManager &animationManager_, int id_, LOOPMETHOD isLoop_ = LOOPMETHOD::JUMP_LOOP, int beginFrame_ = -1, int beginDirection_ = 1);
	void update();
	SDL_Texture* getSprite();
	SDL_Texture* getWhiteSprite();
	SDL_Texture* getBorderSprite();
	bool isFinished();
	void switchDir();
	void setDir(int dir_);
	Vector2<float> getSize();
	Vector2<float> getOrigin();
	void reset(int beginFrame_ = -1, int beginDirection_ = 1);
	int getDirection() const;


	Animation(Animation &anim_) = delete;
	Animation &operator=(Animation &anim_) = delete;
	Animation(Animation &&anim_);
	Animation &operator=(Animation &&anim_);

private:
	std::shared_ptr<TextureArr> m_textures;
	int m_currentFrame;
	int m_direction;
	LOOPMETHOD m_isLoop;
	void animFinished();

};

using Animation_t = std::shared_ptr<Animation>;

#endif