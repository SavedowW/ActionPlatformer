#ifndef STAGE_1_H_
#define STAGE_1_H_

#include "BattleLevel.h"

class Stage1Background : public Background
{
public:
    Stage1Background(TextureManager *textureManager_)
    {
    }

    virtual ~Stage1Background() = default;
};

class Stage1 : public BattleLevel<Stage1Background>
{
public:
    Stage1(Application *application_, const Vector2<float>& size_, int lvlId_);

    virtual void enter() override;

    virtual void update() override;

    virtual ~Stage1() = default;
};

#endif
