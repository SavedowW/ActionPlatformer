#ifndef ANIMATION_MANAGER_H_
#define ANIMATION_MANAGER_H_
#include "TextureManager.h"
#include <fstream>
#include <filesystem>

//Texture array structure
struct TextureArr
{
    TextureArr(unsigned int* tex_, size_t amount_, uint32_t totalDuration_, const std::vector<size_t> &framesData_, int w_, int h_, const Vector2<int> &origin_) :
        m_tex(tex_),
        m_amount(amount_),
        m_w(w_),
        m_h(h_),
        m_totalDuration(totalDuration_),
        m_origin(origin_),
        m_framesData(framesData_)
    {
    }

    unsigned int operator[](const int rhs)
    {
        return m_tex[m_framesData[rhs]];
    }

    //Texture array and required info
    unsigned int* m_tex;
    size_t m_amount;
    int m_w, m_h;
    uint32_t m_totalDuration;
    Vector2<int> m_origin;
    std::vector<size_t> m_framesData;

    //Properly removes texture
    virtual ~TextureArr();
};

struct ContainedAnimationData
{
    std::filesystem::path m_path;
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
    AnimationManager(Renderer &renderer_);
    std::shared_ptr<TextureArr> getTextureArr(ResID id_);
    void preload(const std::string &toPreload_);
    void preload(ResID id_);

    ResID getAnimID(const std::string &animName_) const;

private:
    Renderer &m_renderer;

    std::unordered_map<std::string, ResID> m_ids;
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
    Animation(AnimationManager &animationManager_, ResID id_, LOOPMETHOD isLoop_ = LOOPMETHOD::JUMP_LOOP, int beginFrame_ = -1, int beginDirection_ = 1);
    void update();
    unsigned int getSprite();
    bool isFinished();
    void switchDir();
    void setDir(int dir_);
    Vector2<int> getSize() const;
    Vector2<int> getOrigin() const;
    void reset(int beginFrame_ = -1, int beginDirection_ = 1);
    int getDirection() const;


    //Animation(Animation &anim_) = delete;
    //Animation &operator=(Animation &anim_) = delete;
    //Animation(Animation &&anim_);
    //Animation &operator=(Animation &&anim_);

private:
    std::shared_ptr<TextureArr> m_textures;

    /*
        Cannot be unsigned due to first frame logic:
        on the first frame animation is used, it should show first frame no matter if the update was called or not
    */
    int m_currentFrame;
    int m_direction;
    LOOPMETHOD m_isLoop;
    void animFinished();

};

#endif
