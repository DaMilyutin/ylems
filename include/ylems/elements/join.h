#pragma once
#include <ylems/rules/abstract.h>
#include <variant>
#include <iterator>
#include <ylems/elements/range.h>

namespace ylems
{
    namespace elements
    {

        template<template<typename> typename tag, typename Range1, typename Range2>
        class JoinIterator
        {
        public:
            struct Sentinel {};

            using Stage = std::variant<std::monostate, Range1, Range2>;

            JoinIterator(Range1 r1, Range2 r2)
                : range1_(r1), range2_(r2)
            {
                go_stage1();
            }

            JoinIterator& operator++() { increment(); return *this; }

            auto operator*() const
            {
                switch(stage_.index())
                {
                case 1:  return *std::get<1>(stage_).iterator;
                case 2:  return *std::get<2>(stage_).iterator;
                default: return *std::get<1>(stage_).iterator;
                };
            }

            bool operator==(Sentinel) const { return stage_.index() == 0; }
            bool operator!=(Sentinel) const { return stage_.index() != 0; }

            Stage const& stage() const { return stage_; };
            Stage& mutable_stage() { return stage_; };

            Range1 range1() const { return range1_; }
            Range2 range2() const { return range2_; }

            void go_stage1()
            {
                stage_.emplace<1>(range1_);
                auto const& rng = std::get<1>(stage_);
                if(rng.iterator == rng.sentinel)
                    go_stage2();
            }

            void go_stage2()
            {
                stage_.emplace<2>(range2_);
                auto const& rng = std::get<2>(stage_);
                if(rng.iterator == rng.sentinel)
                    stage_ = std::monostate{};
            }

            void increment()
            {
                switch(stage_.index())
                {
                case 1:
                {
                    auto& range1 = std::get<1>(stage_);
                    ++range1.iterator;
                    if(range1.iterator == range1.sentinel)
                        go_stage2();
                } break;
                case 2:
                {
                    auto& range2 = std::get<2>(stage_);
                    ++range2.iterator;
                    if(range2.iterator == range2.sentinel)
                        stage_ = std::monostate{};
                } break;
                default: return;
                };
            }

        private:
            Range1 range1_;
            Range2 range2_;
            Stage  stage_;
        };



        template<template<typename> typename tag, typename Y1, typename Y2>
        class JoinYield: public ylems::rules::Yield<tag, JoinYield<tag, Y1, Y2>>
        {
        public:

            using Range1 = std::remove_cvref_t<decltype(as_range<tag>(std::declval<Y1>()))>;
            using Range2 = std::remove_cvref_t<decltype(as_range<tag>(std::declval<Y2>()))>;

            using Iterator = JoinIterator<tag, Range1, Range2>;
            using Sentinel = Iterator::Sentinel;

            template<typename T1, typename T2>
            JoinYield(T1&& s1, T2&& s2)
                : y1(FWD(s1)), y2(FWD(s2))
            {}


            Iterator begin() const { return Iterator(as_range<tag>(y1), as_range<tag>(y2)); }
            Sentinel end()   const { return Sentinel{}; }

            Y1 y1;
            Y2 y2;
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
        struct Transfuser<elements::JoinYield<tag, Y1, Y2>, S>
        {
            static auto transfuse(elements::JoinYield<tag, Y1, Y2> const& the_yield, S& the_sink)
            {
                return rules::transfuse(elements::as_range<tag>(the_yield), the_sink);
            }
        };

        template<template<typename> typename tag, typename R1, typename R2, typename S>
        struct Transfuser<elements::RangeWrap<tag, elements::JoinIterator<tag, R1, R2>,
                                                   typename elements::JoinIterator<tag, R1, R2>::Sentinel>,
                          S>
        {
            using Range_t = rules::detail::Range<elements::JoinIterator<tag, R1, R2>, typename elements::JoinIterator<tag, R1, R2>::Sentinel>;

            static Range_t transfuse(elements::RangeWrap<tag, elements::JoinIterator<tag, R1, R2>,
                                                  typename elements::JoinIterator<tag, R1, R2>::Sentinel> the_yield,
                                     S& the_sink)
            {
                auto i = the_yield.begin();
                auto s = the_yield.end();
                auto& stage = i.mutable_stage();
                if(stage.index() == 1)
                {
                    std::get<1>(stage) = rules::transfuse(std::get<1>(stage), the_sink);
                    {
                        auto const& rng1 = std::get<1>(stage);
                        if(rng1.iterator != rng1.sentinel)
                            return {i, s};
                    }
                    i.go_stage2();
                    std::get<2>(stage) = rules::transfuse(std::get<2>(stage), the_sink);
                    return {i, s};
                }
                else if(stage.index() == 2)
                {
                    std::get<2>(stage) = rules::transfuse(std::get<2>(stage), the_sink);
                    return {i, s};
                }
                return the_yield;
            }
        };
    }
}
