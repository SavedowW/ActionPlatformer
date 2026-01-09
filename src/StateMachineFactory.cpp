#include "StateMachineFactory.h"
#include "Core/CoreComponents.h"

namespace SM::Update
{
    UpdateVelInr::UpdateVelInr(
            TimelineProperty<Vector2<float>> &&mulOwnVelUpd_,
            TimelineProperty<Vector2<float>> &&mulOwnDirVelUpd_,
            TimelineProperty<Vector2<float>> &&rawAddVelUpd_,
            TimelineProperty<Vector2<float>> &&mulOwnInrUpd_,
            TimelineProperty<Vector2<float>> &&mulOwnDirInrUpd_,
            TimelineProperty<Vector2<float>> &&rawAddInrUpd_) :
        m_mulOwnVelUpd{std::move(mulOwnVelUpd_)},
        m_mulOwnDirVelUpd{std::move(mulOwnDirVelUpd_)},
        m_rawAddVelUpd{std::move(rawAddVelUpd_)},
        m_mulOwnInrUpd{std::move(mulOwnInrUpd_)},
        m_mulOwnDirInrUpd{std::move(mulOwnDirInrUpd_)},
        m_rawAddInrUpd{std::move(rawAddInrUpd_)}
    {}

    void UpdateVelInr::operator()(const ComponentViewer &viewer_, uint32_t currentFrame_) const
    {
        
        auto &physical = viewer_.get<ComponentPhysical>();
        auto &transform = viewer_.get<ComponentTransform>();

        physical.m_velocity = physical.m_velocity.mulComponents(m_mulOwnVelUpd[currentFrame_])
            + Vector2<int>(transform.m_orientation).mulComponents(m_mulOwnDirVelUpd[currentFrame_])
            + m_rawAddVelUpd[currentFrame_];
        physical.m_inertia = physical.m_inertia.mulComponents(m_mulOwnInrUpd[currentFrame_])
            + Vector2<int>(transform.m_orientation).mulComponents(m_mulOwnDirInrUpd[currentFrame_])
            + m_rawAddInrUpd[currentFrame_];
    }


    VelInrLimit::VelInrLimit(TimelineProperty<Vector2<float>> &&ownVelLimitUpd_,
            TimelineProperty<Vector2<float>> &&ownInrLimitUpd_) :
        m_ownVelLimitUpd{std::move(ownVelLimitUpd_)},
        m_ownInrLimitUpd{std::move(ownInrLimitUpd_)}
    {}

    void VelInrLimit::operator()(const ComponentViewer &viewer_, uint32_t currentFrame_) const
    {
        auto &physical = viewer_.get<ComponentPhysical>();

        // Handle velocity and inertia limits
        physical.m_velocity = utils::clamp(physical.m_velocity, -m_ownVelLimitUpd[currentFrame_], m_ownVelLimitUpd[currentFrame_]);

        physical.m_inertia = utils::clamp(physical.m_inertia, -m_ownInrLimitUpd[currentFrame_], m_ownInrLimitUpd[currentFrame_]);
    }


    GenericPhysics::GenericPhysics(TimelineProperty<Vector2<float>> &&gravity_,
            TimelineProperty<Vector2<float>> &&drag_,
            TimelineProperty<Vector2<float>> &&appliedInertiaMultiplier_,
            TimelineProperty<bool> &&noLanding_,
            TimelineProperty<unsigned int> &&magnetLimit_) :
        m_gravity{std::move(gravity_)},
        m_drag{std::move(drag_)},
        m_appliedInertiaMultiplier{std::move(appliedInertiaMultiplier_)},
        m_noLanding{std::move(noLanding_)},
        m_magnetLimit{std::move(magnetLimit_)}
    {}

    void GenericPhysics::operator()(const ComponentViewer &viewer_, uint32_t currentFrame_) const
    {
        auto &physical = viewer_.get<ComponentPhysical>();
        
        // Handle gravity
        physical.m_gravity = m_gravity[currentFrame_];
        physical.m_drag = m_drag[currentFrame_];
        physical.m_inertiaMultiplier = m_appliedInertiaMultiplier[currentFrame_];
        physical.m_noLanding = m_noLanding[currentFrame_];
        physical.m_magnetLimit = m_magnetLimit[currentFrame_];
    }
} // SM::Body
