#ifndef ANIMATION_MANAGER_H_
#define ANIMATION_MANAGER_H_
#include "Vector2.hpp"
#include "Timer.h"
#include "TimelineProperty.hpp"
#include <filesystem>
#include <unordered_map>
#include <vector>

//Texture array structure
struct TextureArr
{
    TextureArr(std::vector<unsigned int> &&tex_, size_t amount_, Time::NS totalDuration_, TimelineProperty<Time::NS, size_t> &&framesData_, int w_, int h_, const Vector2<int> &origin_) :
        m_tex(std::move(tex_)),
        m_amount(amount_),
        m_w(w_),
        m_h(h_),
        m_totalDuration(totalDuration_),
        m_origin(origin_),
        m_framesData(std::move(framesData_))
    {
    }

    unsigned int operator[](const Time::NS &rhs)
    {
        return m_tex[m_framesData[rhs]];
    }

    //Texture array and required info
    std::vector<unsigned int> m_tex;
    size_t m_amount;
    int m_w, m_h;
    Time::NS m_totalDuration;
    Vector2<int> m_origin;
    TimelineProperty<Time::NS, size_t> m_framesData;

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
    AnimationManager();
    std::shared_ptr<TextureArr> getTextureArr(ResID id_);
    void preload(const std::string &toPreload_);
    void preload(ResID id_);

    ResID getAnimID(const std::string &animName_) const;

private:
    std::unordered_map<std::string, ResID> m_ids;
    std::vector<ContainedAnimationData> m_textureArrs;
};

enum class LOOPMETHOD : uint8_t
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
    enum class Direction : uint8_t {
        FORWARD,
        BACKWARD
    };

    Animation(AnimationManager &animationManager_, ResID id_, LOOPMETHOD isLoop_ = LOOPMETHOD::JUMP_LOOP, const Time::NS &initialTime_ = Time::NS{0});
    void update(Time::NS frameDuration_);
    unsigned int getSprite() const;
    bool isFinished() const noexcept;
    Vector2<int> getSize() const noexcept;
    Vector2<int> getOrigin() const noexcept;
    void reset() noexcept;
    Direction getDirection() const noexcept;

private:
    std::shared_ptr<TextureArr> m_textures;

    /*
        Cannot be unsigned due to first frame logic:
        on the first frame animation is used, it should show first frame no matter if the update was called or not
    */
    Time::NS m_timePassed;
    Direction m_direction = Direction::FORWARD;
    LOOPMETHOD m_isLoop;

};

#endif
