#ifndef ENGINE_ANIMATION_H_
#define ENGINE_ANIMATION_H_

#include "Renderer.h"
#include "TimelineProperty.hpp"
#include "lz4.h"
#include "lz4frame.h"
#include "lz4hc.h"
#include <array>

class EngineAnimation
{
public:
    EngineAnimation();

    bool saveAnimation(const std::string &path_, int blurRange_, Renderer &ren_, int version_);

    bool loadAnimation(const std::string &path_);

    void addFrame(const std::string &path_);

    void setFrame(uint32_t frameId_, int spriteId_);
    void setDuration(uint32_t duration_);
    void scaleToHeight(int height_);

    void generateLayer(size_t layer_, int blurRange_, Renderer &ren_);

    void clear();

    ~EngineAnimation();

//private:
    struct Layer
    {
        std::vector<SDL_Surface *> m_surfaces;
        bool m_isGenerated = false;
        std::string m_layerName;

        void addSurface(SDL_Surface *sur_);
        void clear();
    };

    std::array<Layer, 3> m_layers;

    int m_width = 1;
    int m_height = 1;
    int m_realWidth = 1;
    int m_realHeight = 1;
    int m_frameCount = 0;
    SDL_RWops *m_rw = nullptr;

    TimelinePropertyEditable<int> m_framesData;
    int m_duration = 1;
    Vector2<float> m_origin;

    void saveSurfaceLZ4(SDL_Surface *sur_, Uint32 uncompressed_size_, int max_lz4_size_);
    SDL_Surface *loadSurfaceLZ4();

    char *m_compressionBuffer = nullptr;

    // Create pure white version of surface
    SDL_Surface *toPureWhite(SDL_Surface *sur_, int blurRange_);
    SDL_Surface *toEdge(SDL_Surface *sur_);

    void saveAnimationV1(int blurRange_, Renderer &ren_);
    void saveAnimationV2(int blurRange_, Renderer &ren_);

    void loadAnimationV1();
    void loadAnimationV2();
};

#endif