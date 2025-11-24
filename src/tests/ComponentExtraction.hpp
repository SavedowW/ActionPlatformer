#pragma once
#include "Core/ECS/ComponentRegistry.hpp"
#include "Core/ECS/ComponentExtractor.hpp"
#include "Core/CoreComponents.h"
#include "Core/Vector2.hpp"

void dump3(const ComponentViewer &view_)
{
    const auto &trans = view_.get<ComponentTransform>();
    const auto &phys = view_.get<ComponentPhysical>();
    const auto &ren = view_.get<ComponentAnimationRenderable>();

    std::cout << trans.m_pos << " " << serialize(trans.m_orientation) << "\n  "
            << phys.m_velocity << "\n  "
            << ren.m_animations.size() << " " << std::hex << reinterpret_cast<int64_t>(ren.m_currentAnimation) << std::dec << std::endl;
}

void dump2(const ComponentViewer &view_)
{
    const auto &trans = view_.get<ComponentTransform>();
    const auto &phys = view_.get<ComponentPhysical>();

    std::cout << trans.m_pos << " " << serialize(trans.m_orientation) << "\n  "
            << phys.m_velocity << std::endl;
}

void testCompExtraction()
{
    entt::registry reg;

    auto ent1 = reg.create();
    auto ent2 = reg.create();

    reg.emplace<ComponentTransform>(ent1, Vector2{1, 2}, ORIENTATION::LEFT);
    reg.emplace<ComponentPhysical>(ent1).m_velocity = {1.0f, 0.0f};
    reg.emplace<ComponentAnimationRenderable>(ent1);

    reg.emplace<ComponentTransform>(ent2, Vector2{-1, 0}, ORIENTATION::UNSPECIFIED);
    reg.emplace<ComponentPhysical>(ent2).m_velocity = {-1.0f, 1.3f};

    ComponentRegistry compReg;
    compReg.initExtractorFactory<ComponentTransform>();
    compReg.initExtractorFactory<ComponentPhysical>();
    compReg.initExtractorFactory<ComponentAnimationRenderable>();

    ComponentViewer viewer3;
    ComponentViewer viewer2;

    {
        auto ex1 = compReg.makeExtractor("ComponentTransform");
        auto ex2 = compReg.makeExtractor("ComponentPhysical");
        auto ex3 = compReg.makeExtractor("ComponentAnimationRenderable");
        viewer3.addExtractor(ex1.first, std::move(ex1.second));
        viewer3.addExtractor(ex2.first, std::move(ex2.second));
        viewer3.addExtractor(ex3.first, std::move(ex3.second));
    }

    {
        auto ex1 = compReg.makeExtractor("ComponentTransform");
        auto ex2 = compReg.makeExtractor("ComponentPhysical");
        viewer2.addExtractor(ex1.first, std::move(ex1.second));
        viewer2.addExtractor(ex2.first, std::move(ex2.second));
    }

    viewer3.select(reg, ent1);
    dump3(viewer3);

    viewer2.select(reg, ent2);
    dump2(viewer2);

    viewer2.select(reg, ent1);
    dump2(viewer2);

    viewer3.select(reg, ent2);
    dump3(viewer3);
}
