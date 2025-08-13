#include "AnimationManager.h"
#include "TimelineProperty.hpp"
#include "JsonUtils.hpp"
#include "FilesystemUtils.h"
#include <nlohmann/json.hpp>
#include <SDL3_image/SDL_image.h>
#include <cassert>

AnimationManager::AnimationManager()
{
    auto sprDirectory = Filesystem::getRootDirectory() + "Resources/Animations/";

    std::cout << "=== LISTING FOUND ANIMATIONS ===" << std::endl;
    for (const auto &entry : std::filesystem::recursive_directory_iterator(sprDirectory))
    {
        std::filesystem::path dirpath = entry.path();
        auto parentPath = dirpath.parent_path();
        auto fn = entry.path().filename().replace_extension();
        if (entry.is_regular_file() && dirpath.extension() == ".json" && parentPath.filename() == fn)
        {
            auto path = Filesystem::getRelativePath(sprDirectory, parentPath);
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
    {
        //Logger::print("Texture arr is preloaded\n");
        return m_textureArrs[id_].m_preloaded;
    }
    else if (m_textureArrs[id_].m_texArr.expired())
    {
        //Logger::print("Texture arr does not exist, creating new\n");

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

        uint32_t duration = utils::tryClaim<uint32_t>(animdata, "duration", 1);

        TimelineProperty<int> timelineFileIds;
        std::vector<SDL_Surface*> surfaces;
        std::map<int, size_t> fileIdsToInternal;

        auto idbase = m_textureArrs[id_].m_path;
        idbase.replace_extension();
        std::string midfix = utils::tryClaim<std::string>(animdata, "midfix", "");

        for (auto it = animdata["frames"].cbegin(); it != animdata["frames"].cend(); it++)
        {
            uint32_t key = std::stoi(it.key());
            int fileid = it.value();
            
            if (!fileIdsToInternal.contains(fileid))
            {
                auto imgPath = idbase.string() + midfix + std::to_string(fileid) + ".png";
                surfaces.emplace_back(IMG_Load( imgPath.c_str() ));
                fileIdsToInternal[fileid] = surfaces.size() - 1;
            }

            timelineFileIds.addPair(key, fileid);
        }

        unsigned int *texIds = Renderer::surfacesToTexture(surfaces);

        std::vector<size_t> framesData;
        
        for (uint32_t i = 0; i < duration; ++i)
            framesData.push_back(fileIdsToInternal[timelineFileIds[i]]);

        std::shared_ptr<TextureArr> reqElem(new TextureArr(texIds, surfaces.size(), duration, framesData, surfaces[0]->w, surfaces[0]->h, origin));
        m_textureArrs[id_].m_texArr = reqElem;
        return reqElem;
    }
    else
    {
        //Logger::print("Texture arr already exist\n");
        return m_textureArrs[id_].m_texArr.lock();
    }
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
    catch (std::out_of_range exc_)
    {
        throw std::runtime_error("Failed to find animation " + animName_ + " : " + exc_.what());
    }
}

//Properly release all textures
TextureArr::~TextureArr()
{
    //Logger::print("Release " + intToString(amount) + " textures\n");
    glDeleteTextures(static_cast<int>(m_amount), m_tex);
    delete[] m_tex;
}

Animation::Animation(AnimationManager &animationManager_, ResID id_, LOOPMETHOD isLoop_, int beginFrame_, int beginDirection_) :
    m_currentFrame(beginFrame_),
    m_direction(beginDirection_),
    m_isLoop(isLoop_)
{
    m_textures = animationManager_.getTextureArr(id_);
}

void Animation::update()
{
    if (isFinished())
        animFinished();
    else
        m_currentFrame += m_direction;
}

unsigned int Animation::getSprite()
{
    if (m_currentFrame == -1)
        m_currentFrame = 0;
        
    return (*m_textures)[m_currentFrame];
}

bool Animation::isFinished()
{
    if (m_direction > 0)
        return m_currentFrame == static_cast<int>(m_textures->m_totalDuration - 1);
    else
        return !m_currentFrame;
}

void Animation::switchDir()
{
    m_direction *= -1;
}

void Animation::setDir(int dir_)
{
    assert(dir_ == 1 || dir_ == -1);

    m_direction = dir_;
}

void Animation::reset(int beginFrame_, int beginDirection_)
{
    m_currentFrame = beginFrame_;
    m_direction = beginDirection_;
}

void Animation::animFinished()
{
    switch (m_isLoop)
    {
    case (LOOPMETHOD::NOLOOP):
            m_direction = 0;
        break;

    case (LOOPMETHOD::JUMP_LOOP):
        if (m_currentFrame >= static_cast<int>(m_textures->m_totalDuration - 1))
            m_currentFrame = 0;
        else if (m_currentFrame <= 0)
            m_currentFrame = m_textures->m_totalDuration - 1;
        break;

    case (LOOPMETHOD::SWITCH_DIR_LOOP):
        m_direction *= -1;
        if (m_direction == -1)
            m_currentFrame = m_textures->m_totalDuration - 1;
        else
            m_currentFrame = 0;
        break;
    }
}

Vector2<int> Animation::getSize() const
{
    return {m_textures->m_w, m_textures->m_h};
}

Vector2<int> Animation::getOrigin() const
{
    return m_textures->m_origin;
}

int Animation::getDirection() const
{
    return m_direction;
}

