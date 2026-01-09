#pragma once
#include "StateMachineEx.h"
#include "Core/Vector2.hpp"
#include "Core/TimelineProperty.hpp"

/**
 *  Various state update body components
 */
namespace SM::Update
{
    /**
     *  Velocity multiplier shouldn't be negative since there is an arbitrary number of multiplications depending on framerate
     */
    class UpdateVelInr : public BodyCallable
    {
    public:
        UpdateVelInr(
            TimelineProperty<Vector2<float>> &&mulOwnVelUpd_,
            TimelineProperty<Vector2<float>> &&mulOwnDirVelUpd_,
            TimelineProperty<Vector2<float>> &&rawAddVelUpd_,
            TimelineProperty<Vector2<float>> &&mulOwnInrUpd_,
            TimelineProperty<Vector2<float>> &&mulOwnDirInrUpd_,
            TimelineProperty<Vector2<float>> &&rawAddInrUpd_);

        void operator()(const ComponentViewer &viewer_, uint32_t currentFrame_) const override;

    private:
        const TimelineProperty<Vector2<float>> m_mulOwnVelUpd;
        const TimelineProperty<Vector2<float>> m_mulOwnDirVelUpd;
        const TimelineProperty<Vector2<float>> m_rawAddVelUpd;
        const TimelineProperty<Vector2<float>> m_mulOwnInrUpd;
        const TimelineProperty<Vector2<float>> m_mulOwnDirInrUpd;
        const TimelineProperty<Vector2<float>> m_rawAddInrUpd;
    };

    class VelInrLimit : public BodyCallable
    {
    public:
        VelInrLimit(TimelineProperty<Vector2<float>> &&ownVelLimitUpd_,
            TimelineProperty<Vector2<float>> &&ownInrLimitUpd_);

        void operator()(const ComponentViewer &viewer_, uint32_t currentFrame_) const override;

    private:
        TimelineProperty<Vector2<float>> m_ownVelLimitUpd;
        TimelineProperty<Vector2<float>> m_ownInrLimitUpd;
    };

    /**
     *  Gravity, drag, inertia multiplier, landing and magnet restrictions
     */
    class GenericPhysics : public BodyCallable
    {
    public:
        GenericPhysics(TimelineProperty<Vector2<float>> &&gravity_,
            TimelineProperty<Vector2<float>> &&drag_,
            TimelineProperty<Vector2<float>> &&appliedInertiaMultiplier_,
            TimelineProperty<bool> &&noLanding_,
            TimelineProperty<unsigned int> &&magnetLimit_);

        void operator()(const ComponentViewer &viewer_, uint32_t currentFrame_) const override;

    private:
        const TimelineProperty<Vector2<float>> m_gravity;
        const TimelineProperty<Vector2<float>> m_drag;
        const TimelineProperty<Vector2<float>> m_appliedInertiaMultiplier;
        const TimelineProperty<bool> m_noLanding;
        const TimelineProperty<unsigned int> m_magnetLimit;
    };
}
