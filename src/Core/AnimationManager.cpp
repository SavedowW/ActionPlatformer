#include "AnimationManager.h"
#include "Renderer.h"
#include "JsonUtils.hpp"
#include "FilesystemUtils.h"
#include <nlohmann/json.hpp>
#include "Timer.h"
#include "glad/glad.h"
#include <SDL3_image/SDL_image.h>
#include <cassert>
#include <fstream>

AnimationManager::AnimationManager()
{
    Filesystem::ensureDirectoryRelative("Resources/Animations");
    const std::filesystem::path basePath(Filesystem::getRootDirectory() + "Resources/Animations/");

    std::cout << "=== LISTING FOUND ANIMATIONS ===" << std::endl;
    for (const auto &entry : std::filesystem::recursive_directory_iterator(basePath))
    {
        const std::filesystem::path &dirpath = entry.path();
        auto parentPath = dirpath.parent_path();
        auto fn = entry.path().filename().replace_extension();
        if (entry.is_regular_file() && dirpath.extension() == ".json" && parentPath.filename() == fn)
        {
            auto path = Filesystem::getRelativePath(basePath, parentPath);
            std::cout << path << std::endl;

            ContainedAnimationData cad;
            cad.m_path = dirpath;

            m_textureArrs.push_back(cad);
            m_ids[path] = m_textureArrs.size() - 1;
        }
    }
    std::cout << "=== LISTING ENDS HERE ==="  << std::endl;

    //Load preloading textures
    //preload("Particles/Block");
}

std::shared_ptr<TextureArr> AnimationManager::getTextureArr(ResID id_)
{
    if (m_textureArrs[id_].m_preloaded)
        return m_textureArrs[id_].m_preloaded;

    if (!m_textureArrs[id_].m_texArr.expired())
        return m_textureArrs[id_].m_texArr.lock();

    std::ifstream animjson(m_textureArrs[id_].m_path);
    if (!animjson.is_open())
    {
        std::cout << "Failed to open animation description at \"" << m_textureArrs[id_].m_path << "\"\n";
        return nullptr;
    }

    nlohmann::json animdata = nlohmann::json::parse(animjson);

    Vector2<int> origin(
        utils::tryClaim(animdata, "origin_x", 0),
        utils::tryClaim(animdata, "origin_y", 0));

    const auto duration = utils::tryClaim<Time::NS>(animdata, "duration", Time::NS{1});

    TimelineProperty<Time::NS, size_t> timelineFileIds;
    std::vector<SDL_Surface*> surfaces;
    std::map<int, size_t> fileIdsToInternal;

    auto idbase = m_textureArrs[id_].m_path;
    idbase.replace_extension();
    const auto midfix = utils::tryClaim<std::string>(animdata, "midfix", "");

    for (auto it = animdata["frames"].cbegin(); it != animdata["frames"].cend(); it++)
    {
        Time::NS key = Time::deserialize(it.key());
        int fileid = it.value();
        
        if (!fileIdsToInternal.contains(fileid))
        {
            auto imgPath = idbase.string() + midfix + std::to_string(fileid) + ".png";
            surfaces.emplace_back(IMG_Load( imgPath.c_str() ));
            fileIdsToInternal[fileid] = surfaces.size() - 1;
        }

        timelineFileIds.addPair(key, fileIdsToInternal.at(fileid));
    }

    auto texIds = Renderer::surfacesToTexture(surfaces);

    auto reqElem = std::make_shared<TextureArr>(std::move(texIds), surfaces.size(), duration, std::move(timelineFileIds), surfaces[0]->w, surfaces[0]->h, origin);
    m_textureArrs[id_].m_texArr = reqElem;
    return reqElem;
}

void AnimationManager::preload(ResID id_)
{
    if (m_textureArrs[id_].m_preloaded == nullptr)
    {
        m_textureArrs[id_].m_preloaded = std::shared_ptr<TextureArr>(getTextureArr(id_));
    }
}

void AnimationManager::preload(const std::string &toPreload_)
{
    preload(getAnimID(toPreload_));
}

ResID AnimationManager::getAnimID(const std::string &animName_) const
{
    try
    {
        return m_ids.at(animName_);
    }
    catch (const std::out_of_range &exc_)
    {
        throw std::runtime_error("Failed to find animation " + animName_ + " : " + exc_.what());
    }
}

//Properly release all textures
TextureArr::~TextureArr()
{
    //Logger::print("Release " + intToString(amount) + " textures\n");
    glDeleteTextures(static_cast<int>(m_amount), m_tex.data());
}

Animation::Animation(AnimationManager &animationManager_, ResID id_, LOOPMETHOD isLoop_, const Time::NS &initialTime_) :
    m_timePassed{initialTime_},
    m_isLoop(isLoop_)
{
    m_textures = animationManager_.getTextureArr(id_);
}

void Animation::update(Time::NS frameDuration_)
{
    switch (m_direction)
    {
        case Direction::FORWARD:
        {
            const auto newTime = m_timePassed + frameDuration_;
            if (newTime > m_textures->m_totalDuration)
            {
                if (m_isLoop == LOOPMETHOD::JUMP_LOOP)
                    m_timePassed = newTime - m_textures->m_totalDuration;
                else if (m_isLoop == LOOPMETHOD::SWITCH_DIR_LOOP)
                {
                    m_direction = Direction::BACKWARD;
                    m_timePassed = m_textures->m_totalDuration - (newTime - m_textures->m_totalDuration);
                }
                else
                    m_timePassed = m_textures->m_totalDuration;
            }
            else
                m_timePassed = newTime;

            break;
        }

        case Direction::BACKWARD:
        {
            if (frameDuration_ > m_timePassed)
            {
                if (m_isLoop == LOOPMETHOD::JUMP_LOOP)
                    m_timePassed = m_textures->m_totalDuration - (frameDuration_ - m_timePassed);
                else if (m_isLoop == LOOPMETHOD::SWITCH_DIR_LOOP)
                {
                    m_direction = Direction::FORWARD;
                    m_timePassed = frameDuration_ - m_timePassed;
                }
                else
                    m_timePassed = Time::NS{0};
            }
            else
                m_timePassed -= frameDuration_;
        }
    }

}

unsigned int Animation::getSprite() const
{       
    return (*m_textures)[m_timePassed];
}

bool Animation::isFinished() const noexcept
{
    if (m_isLoop != LOOPMETHOD::NOLOOP)
        return false;

    return (m_direction == Direction::FORWARD && m_timePassed >= m_textures->m_totalDuration || m_direction == Direction::BACKWARD && m_timePassed <= Time::NS{0});
}

Vector2<int> Animation::getSize() const noexcept
{
    return {m_textures->m_w, m_textures->m_h};
}

Vector2<int> Animation::getOrigin() const noexcept
{
    return m_textures->m_origin;
}

void Animation::reset() noexcept
{
    m_timePassed = Time::NS{0};
}

Animation::Direction Animation::getDirection() const noexcept
{
    return m_direction;
}

