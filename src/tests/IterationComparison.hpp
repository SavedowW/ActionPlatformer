#pragma once
#include "Core/Timer.h"
#include "entt/core/type_info.hpp"
#include "entt/entity/fwd.hpp"
#include "entt/meta/meta.hpp"
#include "entt/core/type_info.hpp"
#include <entt/entt.hpp>
#include <iostream>

template<uint64_t I>
struct Comp
{
    uint64_t add(uint64_t arg_) const
    {
        return I + arg_;
    }
int i = 0;
};

class IExtractor
{
public:
    IExtractor(const entt::registry &reg_) :
        m_reg{reg_}
    {}

    virtual uint64_t handle(entt::entity ent_) = 0;

    virtual ~IExtractor() = default;

protected:
    const entt::registry &m_reg;
};

template<typename T>
class Extractor : public IExtractor
{
public:
    Extractor(const entt::registry &reg_) :
        IExtractor(reg_)
    {}

    uint64_t handle(entt::entity ent_) override
    {
        return m_reg.get<T>(ent_).add(rand() % 3);
    }
};

using ExtractorMapHash = std::unordered_map<entt::id_type, std::unique_ptr<IExtractor>>;
using ExtractorMapNames = std::unordered_map<std::string, std::unique_ptr<IExtractor>>;
using ExtractorMapPtrs = std::unordered_map<const std::type_info*, std::unique_ptr<IExtractor>>;

template<typename... Ts>
void addToMap(ExtractorMapHash &map_, const entt::registry &reg_)
{
    (map_.emplace(entt::type_hash<Ts>(), std::make_unique<Extractor<Ts>>(reg_)), ...);
}

template<typename... Ts>
void addToMap(ExtractorMapNames &map_, const entt::registry &reg_)
{
    (map_.emplace(typeid(Ts).name(), std::make_unique<Extractor<Ts>>(reg_)), ...);
}

template<typename... Ts>
void addToMap(ExtractorMapPtrs &map_, const entt::registry &reg_)
{
    (map_.emplace(&typeid(Ts), std::make_unique<Extractor<Ts>>(reg_)), ...);
}

template<int8_t I>
struct Marker
{};

template<typename MarkT, typename... Comps>
uint64_t collect1(const entt::registry &reg_)
{
    auto view = reg_.view<MarkT, Comps...>();
    uint64_t res = 0;

    for (const auto &el : view.each())
    {
        res += (std::get<const Comps&>(el).add(rand() % 3) + ...);
    }

    return res;
}

template<typename MarkT, typename... Comps>
uint64_t collect2(const entt::registry &reg_)
{
    auto view = reg_.view<MarkT>();
    uint64_t res = 0;

    for (const auto& [idx] : view.each())
    {
        res += (reg_.get<Comps>(idx).add(rand() % 3) + ...);
    }

    return res;
}

template<typename MarkT, typename ArgT>
uint64_t collect3OverStaticView(const std::unordered_map<ArgT, std::unique_ptr<IExtractor>> &extractors_, entt::registry &reg_, const std::vector<ArgT>& hashes_)
{
    auto view = reg_.view<MarkT>();

    uint64_t res = 0;

    for (const auto &idx: view)
        for (const auto &hash : hashes_)
            res += extractors_.at(hash)->handle(idx);

    return res;
}

template<typename MarkT, typename ArgT>
uint64_t collect3OverRuntimeView(const std::unordered_map<ArgT, std::unique_ptr<IExtractor>> &extractors_, entt::registry &reg_, const std::vector<ArgT>& hashes_)
{
    entt::runtime_view view;
    view.iterate(*reg_.storage(entt::type_hash<MarkT>()));
    for (const auto& hash : hashes_)
        view.iterate(*reg_.storage(hash));

    uint64_t res = 0;

    for (const auto &idx: view)
        for (const auto &hash : hashes_)
            res += extractors_.at(hash)->handle(idx);

    return res;
}

void cmpIteration()
{
    srand(time(nullptr));
    entt::registry reg;

    ExtractorMapHash extractorsHash;
    ExtractorMapNames extractorsNames;
    ExtractorMapPtrs extractorsPtrs;
    addToMap<Comp<10>, Comp<11>, Comp<12>, Comp<13>, Comp<14>, Comp<15>, Comp<16>, Comp<17>, Comp<18>, Comp<19>,
             Comp<20>, Comp<21>, Comp<22>, Comp<23>, Comp<24>, Comp<25>, Comp<26>, Comp<27>, Comp<28>, Comp<29>,
             Comp<30>, Comp<31>, Comp<32>, Comp<33>, Comp<34>, Comp<35>, Comp<36>, Comp<37>, Comp<38>, Comp<39>>(extractorsHash, reg);
    addToMap<Comp<10>, Comp<11>, Comp<12>, Comp<13>, Comp<14>, Comp<15>, Comp<16>, Comp<17>, Comp<18>, Comp<19>,
             Comp<20>, Comp<21>, Comp<22>, Comp<23>, Comp<24>, Comp<25>, Comp<26>, Comp<27>, Comp<28>, Comp<29>,
             Comp<30>, Comp<31>, Comp<32>, Comp<33>, Comp<34>, Comp<35>, Comp<36>, Comp<37>, Comp<38>, Comp<39>>(extractorsNames, reg);
    addToMap<Comp<10>, Comp<11>, Comp<12>, Comp<13>, Comp<14>, Comp<15>, Comp<16>, Comp<17>, Comp<18>, Comp<19>,
             Comp<20>, Comp<21>, Comp<22>, Comp<23>, Comp<24>, Comp<25>, Comp<26>, Comp<27>, Comp<28>, Comp<29>,
             Comp<30>, Comp<31>, Comp<32>, Comp<33>, Comp<34>, Comp<35>, Comp<36>, Comp<37>, Comp<38>, Comp<39>>(extractorsPtrs, reg);

    for (int i = 0; i < 20000; ++i)
    {
        const auto ent1 = reg.create();
        const auto ent2 = reg.create();
        const auto ent3 = reg.create();

        reg.emplace<Marker<1>>(ent1);
        reg.emplace<Marker<2>>(ent2);
        reg.emplace<Marker<3>>(ent3);

        reg.emplace<Comp<10>>(ent1);
        reg.emplace<Comp<11>>(ent1);
        reg.emplace<Comp<12>>(ent1);
        reg.emplace<Comp<13>>(ent1);
        reg.emplace<Comp<14>>(ent1);
        reg.emplace<Comp<15>>(ent1);
        reg.emplace<Comp<16>>(ent1);
        reg.emplace<Comp<17>>(ent1);
        reg.emplace<Comp<18>>(ent1);
        reg.emplace<Comp<19>>(ent1);

        reg.emplace<Comp<20>>(ent2);
        reg.emplace<Comp<21>>(ent2);
        reg.emplace<Comp<22>>(ent2);
        reg.emplace<Comp<23>>(ent2);
        reg.emplace<Comp<24>>(ent2);
        reg.emplace<Comp<25>>(ent2);
        reg.emplace<Comp<26>>(ent2);
        reg.emplace<Comp<27>>(ent2);
        reg.emplace<Comp<28>>(ent2);
        reg.emplace<Comp<29>>(ent2);

        reg.emplace<Comp<30>>(ent3);
        reg.emplace<Comp<31>>(ent3);
        reg.emplace<Comp<32>>(ent3);
        reg.emplace<Comp<33>>(ent3);
        reg.emplace<Comp<34>>(ent3);
        reg.emplace<Comp<35>>(ent3);
        reg.emplace<Comp<36>>(ent3);
        reg.emplace<Comp<37>>(ent3);
        reg.emplace<Comp<38>>(ent3);
        reg.emplace<Comp<39>>(ent3);
    }

    {
        std::array<uint64_t, 3> results{};
        Timer tmr;
        tmr.begin();
        results = {
            collect1<Marker<1>, Comp<10>, Comp<11>, Comp<12>, Comp<13>, Comp<14>, Comp<15>, Comp<16>, Comp<17>, Comp<18>, Comp<19>>(reg),
            collect1<Marker<2>, Comp<20>, Comp<21>, Comp<22>, Comp<23>, Comp<24>, Comp<25>, Comp<26>, Comp<27>, Comp<28>, Comp<29>>(reg),
            collect1<Marker<3>, Comp<30>, Comp<31>, Comp<32>, Comp<33>, Comp<34>, Comp<35>, Comp<36>, Comp<37>, Comp<38>, Comp<39>>(reg)
        };
        const auto res = tmr.getPassed();
        std::cout << "Fully static result, ms              : " << static_cast<float>(res) / 1'000'000.0f << " / " << results[0] << " / " << results[1] << " / " << results[2] << std::endl;
    }

    {
        std::array<uint64_t, 3> results{};
        Timer tmr;
        tmr.begin();
        results = {
            collect2<Marker<1>, Comp<10>, Comp<11>, Comp<12>, Comp<13>, Comp<14>, Comp<15>, Comp<16>, Comp<17>, Comp<18>, Comp<19>>(reg),
            collect2<Marker<2>, Comp<20>, Comp<21>, Comp<22>, Comp<23>, Comp<24>, Comp<25>, Comp<26>, Comp<27>, Comp<28>, Comp<29>>(reg),
            collect2<Marker<3>, Comp<30>, Comp<31>, Comp<32>, Comp<33>, Comp<34>, Comp<35>, Comp<36>, Comp<37>, Comp<38>, Comp<39>>(reg)
        };
        const auto res = tmr.getPassed();
        std::cout << "Partially static result, ms          : " << static_cast<float>(res) / 1'000'000.0f << " / " << results[0] << " / " << results[1] << " / " << results[2] << std::endl;
    }

    {
        std::array<uint64_t, 3> results{};
        Timer tmr;
        tmr.begin();
        results = {
            collect3OverStaticView<Marker<1>>(extractorsHash, reg, {entt::type_hash<Comp<10>>(), entt::type_hash<Comp<11>>(), entt::type_hash<Comp<12>>(), entt::type_hash<Comp<13>>(), entt::type_hash<Comp<14>>(), entt::type_hash<Comp<15>>(), entt::type_hash<Comp<16>>(), entt::type_hash<Comp<17>>(), entt::type_hash<Comp<18>>(), entt::type_hash<Comp<19>>()}),
            collect3OverStaticView<Marker<2>>(extractorsHash, reg, {entt::type_hash<Comp<20>>(), entt::type_hash<Comp<21>>(), entt::type_hash<Comp<22>>(), entt::type_hash<Comp<23>>(), entt::type_hash<Comp<24>>(), entt::type_hash<Comp<25>>(), entt::type_hash<Comp<26>>(), entt::type_hash<Comp<27>>(), entt::type_hash<Comp<28>>(), entt::type_hash<Comp<29>>()}),
            collect3OverStaticView<Marker<3>>(extractorsHash, reg, {entt::type_hash<Comp<30>>(), entt::type_hash<Comp<31>>(), entt::type_hash<Comp<32>>(), entt::type_hash<Comp<33>>(), entt::type_hash<Comp<34>>(), entt::type_hash<Comp<35>>(), entt::type_hash<Comp<36>>(), entt::type_hash<Comp<37>>(), entt::type_hash<Comp<38>>(), entt::type_hash<Comp<39>>()})
        };
        const auto res = tmr.getPassed();
        std::cout << "Hash-based result over static view, ms      : " << static_cast<float>(res) / 1'000'000.0f << " / " << results[0] << " / " << results[1] << " / " << results[2] << std::endl;
    }

    {
        std::array<uint64_t, 3> results{};
        Timer tmr;
        tmr.begin();
        results = {
            collect3OverRuntimeView<Marker<1>>(extractorsHash, reg, {entt::type_hash<Comp<10>>(), entt::type_hash<Comp<11>>(), entt::type_hash<Comp<12>>(), entt::type_hash<Comp<13>>(), entt::type_hash<Comp<14>>(), entt::type_hash<Comp<15>>(), entt::type_hash<Comp<16>>(), entt::type_hash<Comp<17>>(), entt::type_hash<Comp<18>>(), entt::type_hash<Comp<19>>()}),
            collect3OverRuntimeView<Marker<2>>(extractorsHash, reg, {entt::type_hash<Comp<20>>(), entt::type_hash<Comp<21>>(), entt::type_hash<Comp<22>>(), entt::type_hash<Comp<23>>(), entt::type_hash<Comp<24>>(), entt::type_hash<Comp<25>>(), entt::type_hash<Comp<26>>(), entt::type_hash<Comp<27>>(), entt::type_hash<Comp<28>>(), entt::type_hash<Comp<29>>()}),
            collect3OverRuntimeView<Marker<3>>(extractorsHash, reg, {entt::type_hash<Comp<30>>(), entt::type_hash<Comp<31>>(), entt::type_hash<Comp<32>>(), entt::type_hash<Comp<33>>(), entt::type_hash<Comp<34>>(), entt::type_hash<Comp<35>>(), entt::type_hash<Comp<36>>(), entt::type_hash<Comp<37>>(), entt::type_hash<Comp<38>>(), entt::type_hash<Comp<39>>()})
        };
        const auto res = tmr.getPassed();
        std::cout << "Hash-based result over runtime view, ms      : " << static_cast<float>(res) / 1'000'000.0f << " / " << results[0] << " / " << results[1] << " / " << results[2] << std::endl;
    }

    {
        //entt::type_info(std::in_place_type_t<Comp<11>>{}).index();
        //entt::runtime_view view{};
        //view.iterate(reg.storage<Comp<10>>());
        /*view.iterate(*reg.storage(entt::type_hash<Comp<10>>()));
        auto cnt = idx;
        view.each([](auto arg) {
            std::cout << typeid(arg).name() << std::endl;
        });*/
    }
}
