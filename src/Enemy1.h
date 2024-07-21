#ifndef ENEMY_1_H_
#define ENEMY_1_H_

#include "Action.hpp"
#include "CollisionArea.h"
#include <map>
#include "ActionCharacter.hpp"


enum class Enemy1States
{
    IDLE = 0,
    FLOAT = 1,
    NONE
};

const inline std::map<Enemy1States, const char *> Enemy1StatesNames{
    {Enemy1States::IDLE, "IDLE"},
    {Enemy1States::FLOAT, "FLOAT"},
};

class Enemy1 : public ActionCharacter<Enemy1, Enemy1States>
{
public:
    Enemy1(Application &application_, const CollisionArea &cldArea_);
    virtual bool getNoUpwardLanding() override;

protected:
    friend CharacterGenericAction;

    virtual void loadAnimations(Application &application_) override;

};

#endif
