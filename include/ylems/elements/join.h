#pragma once
#include <ylems/rules/abstract.h>
#include <ylems/utilities/tuple_fun.h>
#include <variant>

namespace ylems
{
    namespace elements
    {
        template<template<typename> typename tag, typename Y1, typename Y2>
        class JoinYield: public ylems::rules::Yield<tag, JoinYield<tag, Y1, Y2>>
        {
            using I1 = std::remove_cvref_t<decltype(std::begin(std::declval<Y1>()))>;
            using S1 = std::remove_cvref_t<decltype(std::end(std::declval<Y1>()))>;

            using I2 = std::remove_cvref_t<decltype(std::begin(std::declval<Y2>()))>;
            using S2 = std::remove_cvref_t<decltype(std::end(std::declval<Y2>()))>;

            using RV1 = decltype(*std::declval<I1>());
            using RV2 = decltype(*std::declval<I2>());

            using V1 = std::remove_cvref_t<RV1>;
            using V2 = std::remove_cvref_t<RV2>;

            static_assert(std::is_same_v<V1, V2>, "joined yields must return matching type!");
            using value_type = std::conditional_t<std::is_same_v<RV1, RV2>, RV1, V1>;


            struct Range1
            {
                I1 b;
                S1 e;
            };

            struct Range2
            {
                I2 b;
                S2 e;
            };

        public:


            template<typename T1, typename T2>
            JoinYield(T1&& s1, T2&& s2)
                : y1(FWD(s1)), y2(FWD(s2))
            {
                //ylems::concepts::NotAYield<tag, JoinYield<tag, Y1, Y2>> x = true;
                auto xx =  std::is_same_v<JoinYield<tag, Y1, Y2>, std::remove_cvref_t<decltype(*this)>>;
                ylems::concepts::IsAYield<tag, JoinYield<tag, Y1, Y2>>  y = true;
            }

            struct Sentinel {};

            class Iterator
            {
            public:
                Iterator(JoinYield const& j)
                    : j_(j)
                {
                    go_stage1();
                }

                Iterator& operator++() { increment(); return *this; }

                value_type operator*() const
                {
                    switch(stage_.index())
                    {
                    case 1:  return *std::get<Range1>(stage_).b;
                    case 2:  return *std::get<Range2>(stage_).b;
                    default: return {};
                    };
                }

                bool operator==(Sentinel) const { return stage_.index() == 0; }
                bool operator!=(Sentinel) const { return stage_.index() != 0; }


            private:

                void go_stage1()
                {
                    stage_.emplace<Range1>(std::begin(j_.y1), std::end(j_.y1));
                    auto const& rng = std::get<Range1>(stage_);
                    if(rng.b == rng.e)
                        go_stage2();
                }

                void go_stage2()
                {
                    stage_.emplace<Range2>(std::begin(j_.y2), std::end(j_.y2));
                    auto const& rng = std::get<Range2>(stage_);
                    if(rng.b == rng.e)
                        stage_ = std::monostate{};
                }

                void increment()
                {
                    switch(stage_.index())
                    {
                    case 1:
                    {
                        auto& range1 = std::get<Range1>(stage_);
                        ++range1.b;
                        if(range1.b == range1.e)
                            go_stage2();
                    } break;
                    case 2:
                    {
                        auto& range2 = std::get<Range2>(stage_);
                        ++range2.b;
                        if(range2.b == range2.e)
                            stage_ = std::monostate{};
                    } break;
                    default: return;
                    };
                }

                JoinYield const& j_;
                std::variant<std::monostate, Range1, Range2> stage_;
            };

            Iterator begin() const { return Iterator(*this); }
            Sentinel end()   const { return Sentinel{}; }

            Y1 y1;
            Y1 y2;
        };

        template<template<typename> typename tag, typename Y1, typename Y2>
        JoinYield<tag, Y1, Y2> join(rules::Yield<tag, Y1>&& y1, rules::Yield<tag, Y2>&& y2)
        {
            return {FWD(y1)._get_(), FWD(y2)._get_()};
        }

        template<template<typename> typename tag, typename Y1, typename Y2>
        JoinYield<tag, Y1 const&, Y2> join(rules::Yield<tag, Y1> const& y1, rules::Yield<tag, Y2>&& y2)
        {
            return {y1._get_(), FWD(y2)._get_()};
        }

        template<template<typename> typename tag, typename Y1, typename Y2>
        JoinYield<tag, Y1, Y2 const&> join(rules::Yield<tag, Y1>&& y1, rules::Yield<tag, Y2> const& y2)
        {
            return {FWD(y1)._get_(), y2._get_()};
        }

        template<template<typename> typename tag, typename Y1, typename Y2>
        JoinYield<tag, Y1 const&, Y2 const&> join(rules::Yield<tag, Y1> const& y1, rules::Yield<tag, Y2> const& y2)
        {
            return {y1._get_(), y2._get_()};
        }

        template<template<typename> typename tag, typename... Ys, typename Yn>
        auto join(rules::Yield<tag, Ys>&&... ys, rules::Yield<tag, Yn> const& yn)
        {
            return join(join(FWD(ys)...), FWD(yn));
        }
    }

    namespace rules
    {
        template<template<typename> typename tag, typename Y1, typename Y2, typename S>
        bool meld(elements::JoinYield<tag, Y1, Y2> const& yield, Sink<tag, S>& sink)
        {
            auto& the_sink = sink._get_();
            auto& the_yield = yield._get_();
            for(auto&& e: the_yield.y1)
                if(!the_sink.consume(e))
                    return false; // if sink forced to stop
            for(auto&& e: the_yield.y2)
                if(!the_sink.consume(e))
                    return false; // if sink forced to stop
            return true;
        }
    }
}
