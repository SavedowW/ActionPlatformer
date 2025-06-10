#include "EngineAnimation.h"
#include <SDL_Image.h>

bool isPixelInSurface(int x_, int y_, SDL_Surface *sur_)
{
    return x_ > 0 && y_ > 0 && x_ < sur_->w && y_ < sur_->h;
}

uint8_t *getPixel(int x_, int y_, SDL_Surface *sur_)
{
    return ((uint8_t*)sur_->pixels + sur_->pitch * y_ + x_ * 4);
}

uint8_t getPixelAlphaRaw(int x_, int y_, SDL_Surface *sur_)
{
    return getPixel(x_, y_, sur_)[3];
}

uint8_t getPixelAlpha(int x_, int y_, SDL_Surface *sur_)
{
    if (isPixelInSurface(x_, y_, sur_))
        return getPixelAlphaRaw(x_, y_, sur_);
    else
        return 0;
}


EngineAnimation::EngineAnimation()
{
    m_framesData.addPropertyValue(0, std::move(0));

    m_layers[0].m_layerName = "RGB";
    m_layers[0].m_isGenerated = true;

    m_layers[1].m_layerName = "Pure White";
    m_layers[2].m_layerName = "Light Edge";
}

bool EngineAnimation::saveAnimation(const std::string &path_, int blurRange_, Renderer &ren_, int version_)
{
    
    std::cout << "Saving as version " << version_ << std::endl;

    switch (version_)
    {
    case 1:
        m_rw = SDL_RWFromFile(path_.c_str(), "w+b");
        SDL_RWwrite(m_rw, &version_, sizeof(version_), 1);
        saveAnimationV1(blurRange_, ren_);
        SDL_RWclose(m_rw);
        break;

    case 2:
        m_rw = SDL_RWFromFile(path_.c_str(), "w+b");
        SDL_RWwrite(m_rw, &version_, sizeof(version_), 1);
        saveAnimationV2(blurRange_, ren_);
        SDL_RWclose(m_rw);
        break;

    default:
        std::cout << "Cannot save: selected version is not supported\n";
        return false;
    }

    return true;
}

bool EngineAnimation::loadAnimation(const std::string &path_)
{
    m_rw = SDL_RWFromFile(path_.c_str(), "r+b");

    if (!m_rw)
    {
        std::cout << "Failed to open file \"" << path_ << "\"\n";
        return false;
    }

    int version = 1;

    SDL_RWread(m_rw, &version, sizeof(version), 1);

    switch(version)
    {
    case (1):
        loadAnimationV1();
        break;
    
    case (2):
        loadAnimationV2();
        break;

    default:
        std::cout << "Attempting to load unsupported animation version: " << version << std::endl;
        SDL_RWclose(m_rw);
        return false;
    }

    SDL_RWclose(m_rw);

    for (auto &el : m_layers)
    {
        while (el.m_surfaces.size() < m_frameCount)
            el.m_surfaces.push_back(nullptr);
    }

    return true;
}

void EngineAnimation::addFrame(const std::string &path_)
{
    SDL_Surface *frame = IMG_Load(path_.c_str());
    m_layers[0].addSurface(frame);
    for (int i = 1; i < m_layers.size(); ++i)
        m_layers[i].addSurface(nullptr);


    if (m_frameCount == 0)
    {
        m_width = frame->w;
        m_height = frame->h;
        m_realWidth = m_width;
        m_realHeight = m_height;
    }

    m_frameCount++;
}

void EngineAnimation::setFrame(uint32_t frameId_, int spriteId_)
{
    m_framesData.addPropertyValue(frameId_, std::move(spriteId_));
}

void EngineAnimation::setDuration(uint32_t duration_)
{
    m_duration = duration_;
}

void EngineAnimation::scaleToHeight(int height_)
{
    float scale = (float)m_realHeight / (float)height_;
    m_height = height_;
    m_width = m_realWidth / scale;
}

void EngineAnimation::generateLayer(size_t layer_, int blurRange_, Renderer &ren_)
{
    if (layer_ == 1)
    {
        std::cout << "Generating " + m_layers[layer_].m_layerName << " layer\n";
        for (int i = 0; i < m_frameCount; ++i)
        {
            std::cout << "Sprite " << i + 1 << " / " << m_frameCount << std::endl;

            if (m_layers[layer_].m_surfaces[i])
                SDL_FreeSurface(m_layers[layer_].m_surfaces[i]);
            
            m_layers[layer_].m_surfaces[i] = toPureWhite(m_layers[0].m_surfaces[i], blurRange_);
        }

        m_layers[layer_].m_isGenerated = true;
    }
    else if (layer_ == 2)
    {
        std::cout << "Generating " + m_layers[layer_].m_layerName << " layer\n";
        for (int i = 0; i < m_frameCount; ++i)
        {
            std::cout << "Sprite " << i + 1 << " / " << m_frameCount << std::endl;

            if (m_layers[layer_].m_surfaces[i])
                SDL_FreeSurface(m_layers[layer_].m_surfaces[i]);
            
            m_layers[layer_].m_surfaces[i] = toEdge(m_layers[0].m_surfaces[i]);
        }

        m_layers[layer_].m_isGenerated = true;
    }
    else
    {
        throw std::string("No passed layer");
    }
}

void EngineAnimation::clear()
{
    for (auto &el : m_layers)
        el.clear();

    m_layers[0].m_isGenerated = true;

    m_width = 1;
    m_height = 1;
    m_realWidth = 1;
    m_realHeight = 1;
    m_frameCount = 0;
    m_rw = nullptr;
    m_framesData.clear();
    m_duration = 1;
    m_origin = {0.0f, 0.0f};
    m_compressionBuffer = nullptr;
}

EngineAnimation::~EngineAnimation()
{
    for (auto &el : m_layers)
        el.clear();
}

void EngineAnimation::saveSurfaceLZ4(SDL_Surface *sur_, Uint32 uncompressed_size_, int max_lz4_size_)
{
    SDL_Surface *surcpy = SDL_ConvertSurfaceFormat(sur_, SDL_PIXELFORMAT_ARGB8888, 0);

    const char* uncompressed_buffer = (const char*)(surcpy->pixels);
    
    int true_size = LZ4_compress_HC(uncompressed_buffer, m_compressionBuffer,
                                uncompressed_size_, max_lz4_size_,
                                LZ4HC_CLEVEL_MAX);

    auto sz = surcpy->h * surcpy->w;
    SDL_RWwrite(m_rw, &(true_size), sizeof(true_size), 1);
    auto res = SDL_RWwrite(m_rw, m_compressionBuffer, 1, true_size);

    if (res < true_size)
    {
        std::cout << "Writing " << true_size << " bytes\n";
        std::cout << res << std::endl;
        std::cout << SDL_GetError() << std::endl;
    }

    SDL_FreeSurface(surcpy);
}

SDL_Surface *EngineAnimation::loadSurfaceLZ4()
{
    SDL_Surface *tar = SDL_CreateRGBSurface(0, m_realWidth, m_realHeight, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    Uint32 uncompressed_size = tar->w * tar->h * tar->format->BytesPerPixel;

    int compressed_size;
    SDL_RWread (m_rw, &compressed_size, sizeof(compressed_size), 1);

    char* compressed_buffer = new char[compressed_size];
    auto res = SDL_RWread(m_rw, compressed_buffer, 1, compressed_size);
    if (res == 0)
    {
        std::cout << res << std::endl;
        std::cout << SDL_GetError() << std::endl;
        SDL_FreeSurface(tar);
        delete [] compressed_buffer;
        return nullptr;
    }

    char* uncompressed_buffer = (char*)(tar->pixels);
    LZ4_decompress_safe(compressed_buffer, uncompressed_buffer, compressed_size, uncompressed_size);


    delete [] compressed_buffer;
    return tar;
}

SDL_Surface *EngineAnimation::toPureWhite(SDL_Surface *sur_, int blurRange_)
{
    SDL_Surface *nsur = SDL_CreateRGBSurface(0, sur_->w, sur_->h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    SDL_FillRect(nsur, nullptr, SDL_MapRGBA(nsur->format, 255, 255, 255, 0));

    for (int y = 0; y < nsur->h; ++y)
    {
        for (int x = 0; x < nsur->w; ++x)
        {
            uint8_t *pixel1 = getPixel(x, y, sur_);
            uint8_t *pixel2 = getPixel(x, y, nsur);

            int minx = std::max(0, x - blurRange_);
            int maxx = std::min(sur_->w - 1, x + blurRange_);
            int miny = std::max(0, y - blurRange_);
            int maxy = std::min(sur_->h - 1, y + blurRange_);

            utils::Average<int> avg;

            for (auto x1 = minx; x1 <= maxx; ++x1)
            {
                for (auto y1 = miny; y1 <= maxy; ++y1)
                {
                    uint8_t *pixtmp = ((uint8_t*)sur_->pixels + sur_->pitch * y1 + x1 * 4);
                    avg += getPixelAlphaRaw(x1, y1, sur_);
                }
            }

            pixel2[3] = std::max(uint8_t(avg), pixel1[3]);
        }
    }

    return nsur;
}

SDL_Surface *EngineAnimation::toEdge(SDL_Surface *sur_)
{
    SDL_Surface *nsur = SDL_CreateRGBSurface(0, sur_->w, sur_->h, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    SDL_FillRect(nsur, nullptr, SDL_MapRGBA(nsur->format, 255, 255, 255, 0));

    for (int y = 0; y < nsur->h; ++y)
    {
        for (int x = 0; x < nsur->w; ++x)
        {
            uint8_t *pixel1 = getPixel(x, y, sur_);
            uint8_t *pixel2 = getPixel(x, y, nsur);
            
            if (pixel1[3] > 0)
            {
                utils::Average<int> avg;

                avg += (255 - getPixelAlpha(x, y - 1, sur_)) * 3.0f;
                avg += (255 - getPixelAlpha(x, y - 2, sur_)) * 1.5f;
                avg += (255 - getPixelAlpha(x - 1, y, sur_)) / 3.0f;
                avg += (255 - getPixelAlpha(x - 2, y, sur_)) / 3.0f;
                avg += (255 - getPixelAlpha(x + 1, y, sur_)) / 3.0f;
                avg += (255 - getPixelAlpha(x + 2, y, sur_)) / 3.0f;
                avg += (255 - getPixelAlpha(x - 1, y - 1, sur_)) / 3.0f;
                avg += (255 - getPixelAlpha(x + 1, y - 1, sur_)) / 3.0f;

                pixel2[3] = avg;
            }
        }
    }

    return nsur;
}

void EngineAnimation::Layer::addSurface(SDL_Surface *sur_)
{
    m_surfaces.push_back(sur_);
}

void EngineAnimation::Layer::clear()
{
    for (auto *el : m_surfaces)
        SDL_FreeSurface(el);

    m_surfaces.clear();

    m_isGenerated = false;
}

void EngineAnimation::saveAnimationV1(int blurRange_, Renderer &ren_)
{
    Uint32 uncompressed_size = m_realWidth * m_realHeight * 4;
    int max_lz4_size = LZ4_compressBound (uncompressed_size);
    m_compressionBuffer = new char[max_lz4_size];

    SDL_RWwrite(m_rw, &m_width, sizeof(m_width), 1);
    SDL_RWwrite(m_rw, &m_height, sizeof(m_height), 1);
    SDL_RWwrite(m_rw, &m_realWidth, sizeof(m_realWidth), 1);
    SDL_RWwrite(m_rw, &m_realHeight, sizeof(m_realHeight), 1);
    SDL_RWwrite(m_rw, &m_frameCount, sizeof(m_frameCount), 1);

    SDL_RWwrite(m_rw, &m_origin.x, sizeof(m_origin.x), 1);
    SDL_RWwrite(m_rw, &m_origin.y, sizeof(m_origin.y), 1);

    auto frameDataLen = m_framesData.getValuesCount();
    SDL_RWwrite(m_rw, &frameDataLen, sizeof(frameDataLen), 1);
    for (int i = 0; i < frameDataLen; ++i)
    {
        auto &data = m_framesData.getValuePair(i);
        SDL_RWwrite(m_rw, &data.first, sizeof(data.first), 1);
        SDL_RWwrite(m_rw, &data.second, sizeof(data.second), 1);
    }
    SDL_RWwrite(m_rw, &m_duration, sizeof(m_duration), 1);

    auto layersToSave = {0, 1};

    for (auto &el : layersToSave)
    {
        if (!m_layers[el].m_isGenerated)
            throw std::string("Saving ungenerated layer");
        std::cout << "Saving layer " << m_layers[el].m_layerName << std::endl;

        for (int i = 0; i < m_frameCount; ++i)
        {
            std::cout << "Sprite " << i + 1 << " / " << m_frameCount << std::endl;
            saveSurfaceLZ4(m_layers[el].m_surfaces[i], uncompressed_size, max_lz4_size);
        }
    }

    std::cout << "DONE\n";

    delete [] m_compressionBuffer;
}

void EngineAnimation::saveAnimationV2(int blurRange_, Renderer &ren_)
{
    Uint32 uncompressed_size = m_realWidth * m_realHeight * 4;
    int max_lz4_size = LZ4_compressBound (uncompressed_size);
    m_compressionBuffer = new char[max_lz4_size];

    SDL_RWwrite(m_rw, &m_width, sizeof(m_width), 1);
    SDL_RWwrite(m_rw, &m_height, sizeof(m_height), 1);
    SDL_RWwrite(m_rw, &m_realWidth, sizeof(m_realWidth), 1);
    SDL_RWwrite(m_rw, &m_realHeight, sizeof(m_realHeight), 1);
    SDL_RWwrite(m_rw, &m_frameCount, sizeof(m_frameCount), 1);

    SDL_RWwrite(m_rw, &m_origin.x, sizeof(m_origin.x), 1);
    SDL_RWwrite(m_rw, &m_origin.y, sizeof(m_origin.y), 1);

    auto frameDataLen = m_framesData.getValuesCount();
    SDL_RWwrite(m_rw, &frameDataLen, sizeof(frameDataLen), 1);
    for (int i = 0; i < frameDataLen; ++i)
    {
        auto &data = m_framesData.getValuePair(i);
        SDL_RWwrite(m_rw, &data.first, sizeof(data.first), 1);
        SDL_RWwrite(m_rw, &data.second, sizeof(data.second), 1);
    }
    SDL_RWwrite(m_rw, &m_duration, sizeof(m_duration), 1);

    auto layersToSave = {0, 1, 2};

    for (auto &el : layersToSave)
    {
        if (!m_layers[el].m_isGenerated)
            throw std::string("Saving ungenerated layer");
        std::cout << "Saving layer " << m_layers[el].m_layerName << std::endl;

        for (int i = 0; i < m_frameCount; ++i)
        {
            std::cout << "Sprite " << i + 1 << " / " << m_frameCount << std::endl;
            saveSurfaceLZ4(m_layers[el].m_surfaces[i], uncompressed_size, max_lz4_size);
        }
    }

    std::cout << "DONE\n";

    delete [] m_compressionBuffer;
}

void EngineAnimation::loadAnimationV1()
{
    SDL_RWread(m_rw, &m_width, sizeof(m_width), 1);
    SDL_RWread(m_rw, &m_height, sizeof(m_height), 1);
    SDL_RWread(m_rw, &m_realWidth, sizeof(m_realWidth), 1);
    SDL_RWread(m_rw, &m_realHeight, sizeof(m_realHeight), 1);
    SDL_RWread(m_rw, &m_frameCount, sizeof(m_frameCount), 1);

    SDL_RWread(m_rw, &m_origin.x, sizeof(m_origin.x), 1);
    SDL_RWread(m_rw, &m_origin.y, sizeof(m_origin.y), 1);

    int frameDataLen;
    SDL_RWread(m_rw, &frameDataLen, sizeof(frameDataLen), 1);
    for (int i = 0; i < frameDataLen; ++i)
    {
        uint32_t timeMark;
        int value;
        SDL_RWread(m_rw, &timeMark, sizeof(timeMark), 1);
        SDL_RWread(m_rw, &value, sizeof(value), 1);
        m_framesData.addPropertyValue(timeMark, std::move(value));
    }
    SDL_RWread(m_rw, &m_duration, sizeof(m_duration), 1);


    auto layersToLoad = {0, 1};

    for (auto &el : layersToLoad)
    {
        for (int i = 0; i < m_frameCount; ++i)
        {
            SDL_Surface *tmp = loadSurfaceLZ4();
            m_layers[el].addSurface(tmp);
        }
        m_layers[el].m_isGenerated = true;
    }
}

void EngineAnimation::loadAnimationV2()
{
    SDL_RWread(m_rw, &m_width, sizeof(m_width), 1);
    SDL_RWread(m_rw, &m_height, sizeof(m_height), 1);
    SDL_RWread(m_rw, &m_realWidth, sizeof(m_realWidth), 1);
    SDL_RWread(m_rw, &m_realHeight, sizeof(m_realHeight), 1);
    SDL_RWread(m_rw, &m_frameCount, sizeof(m_frameCount), 1);

    SDL_RWread(m_rw, &m_origin.x, sizeof(m_origin.x), 1);
    SDL_RWread(m_rw, &m_origin.y, sizeof(m_origin.y), 1);

    int frameDataLen;
    SDL_RWread(m_rw, &frameDataLen, sizeof(frameDataLen), 1);
    for (int i = 0; i < frameDataLen; ++i)
    {
        uint32_t timeMark;
        int value;
        SDL_RWread(m_rw, &timeMark, sizeof(timeMark), 1);
        SDL_RWread(m_rw, &value, sizeof(value), 1);
        m_framesData.addPropertyValue(timeMark, std::move(value));
    }
    SDL_RWread(m_rw, &m_duration, sizeof(m_duration), 1);

    auto layersToLoad = {0, 1, 2};

    for (auto &el : layersToLoad)
    {
        for (int i = 0; i < m_frameCount; ++i)
        {
            SDL_Surface *tmp = loadSurfaceLZ4();
            m_layers[el].addSurface(tmp);
        }
        m_layers[el].m_isGenerated = true;
    }
}
