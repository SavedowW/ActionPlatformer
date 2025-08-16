#ifndef STAGE_1_H_
#define STAGE_1_H_

#include "BattleLevel.h"

class Stage1 : public BattleLevel
{
public:
    Stage1(const Vector2<int>& size_, int lvlId_);

    virtual ~Stage1() = default;
};

#endif
