#pragma once
#include <ylems/rules/abstract.h>
#include <variant>
#include <iterator>
#include <ylems/elements/range.h>

#include <ylems/utilities/Storage.h>

#include <array>

namespace ylems
{
    namespace elements
    {
        template<template<typename> typename tag, typename Range1, typename Storage>
        class CycleIterator
        {
        public:
            struct Sentinel {};

            using Range2 = std::remove_cvref_t<decltype(as_range<tag>(std::declval<Storage>()))>;

            using Stage = std::variant<std::monostate, Range1, Range1, Range2>;

            CycleIterator(Range1 r1, Storage& s)
                : range1_(r1), storage_(s)
            {
                go_stage1();
            }

            CycleIterator& operator++() { increment(); return *this; }

            auto operator*() const
            {
                switch(stage_.index())
                {
                case 1:  storage_.push_back(*std::get<1>(stage_).iterator); return storage_.back();
                case 2:  return *std::get<2>(stage_).iterator;
                case 3:  return *std::get<3>(stage_).iterator;
                default: return *std::get<1>(stage_).iterator;
                };
            }

            bool operator==(Sentinel) const { return stage_.index() == 0; }
            bool operator!=(Sentinel) const { return stage_.index() != 0; }

            Stage const& stage() const { return stage_; };

            Range1   range1() const { return range1_; }
            Storage& storage() { return storage_; }

        private:
            void go_stage1()
            {
                stage_.emplace<1>(range1_);
                auto const& rng = std::get<1>(stage_);
                if(rng.iterator == rng.sentinel)
                    go_stage3();
            }

            void go_stage2()
            {
                stage_.emplace<2>(std::get<1>(stage_));
                auto const& rng = std::get<2>(stage_);
                if(rng.iterator == rng.sentinel)
                    go_stage3();
            }

            void go_stage3()
            {
                stage_.emplace<3>(as_range<tag>(storage_));
                auto const& rng = std::get<3>(stage_);
                if(rng.iterator == rng.sentinel)
                    stage_ = std::monostate{};
            }

            void increment()
            {
                switch(stage_.index())
                {
                case 1:
                {
                    auto& rng = std::get<1>(stage_);
                    if(storage_.full())
                        go_stage2();
                    ++rng.iterator;
                    if(rng.iterator == rng.sentinel)
                        go_stage3();
                } break;
                case 2:
                {
                    auto& rng = std::get<2>(stage_);
                    ++rng.iterator;
                    if(rng.iterator == rng.sentinel)
                        go_stage3();
                } break;
                case 3:
                {
                    auto& rng = std::get<3>(stage_);
                    ++rng.iterator;
                    if(rng.iterator == rng.sentinel)
                        stage_ = std::monostate{};
                } break;
                default: return;
                };
            }

            Range1          range1_;
            Storage         storage_;
            Stage           stage_;
        };


        template<template<typename> typename tag, typename Y, typename Storage>
        class CycleYield: public ylems::rules::Yield<tag, CycleYield<tag, Y, Storage>>
        {
        public:
            using Range1 = std::remove_cvref_t<decltype(as_range<tag>(std::declval<Y>()))>;

            using Iterator = CycleIterator<tag, Range1, Storage&>;
            using Sentinel = Iterator::Sentinel;

            template<typename T, typename S>
            CycleYield(T&& y, S&& s)
                : yield(FWD(y)), storage(FWD(s))
            {}

            template<typename T>
            CycleYield(T&& y)
                : yield(FWD(y)), storage()
            {}


            Iterator begin() const { return Iterator(as_range<tag>(yield), storage); }
            Sentinel end()   const { return Sentinel{}; }

            Y               yield;
            Storage mutable storage;
        };

        template<template<typename> typename tag, typename Y, typename S>
        CycleYield<tag, Y, S&>
            cycle(rules::Yield<tag, Y>&& yield, S& storage)
        {
            return {FWD(yield)._get_(), storage};
        }

        template<template<typename> typename tag, typename Y, typename S>
        CycleYield<tag, Y, S>
            cycle(rules::Yield<tag, Y>&& yield, S&& storage)
        {
            return {FWD(yield)._get_(), FWD(storage)};
        }

        template<template<typename> typename tag, typename Y, typename S>
        CycleYield<tag, Y const&, S&>
            cycle(rules::Yield<tag, Y> const& yield, S& storage)
        {
            return {yield._get_(), storage};
        }

        template<template<typename> typename tag, typename Y, typename S>
        CycleYield<tag, Y const&, S>
            cycle(rules::Yield<tag, Y> const& yield, S&& storage)
        {
            return {yield._get_(), FWD(storage)};
        }

        template<template<typename> typename tag, typename Y>
        CycleYield<tag, Y,
                        Storage<std::vector<rules::element_type_t<Y>>>>
            cycle(rules::Yield<tag, Y>&& yield)
        {
            return {FWD(yield)._get_()};
        }

        template<template<typename> typename tag, size_t N, typename Y>
        CycleYield<tag, Y,
                        Storage<std::array<rules::element_type_t<Y>, N>>>
            cycle(rules::Yield<tag, Y>&& yield)
        {
            return {FWD(yield)._get_()};
        }

        template<template<typename> typename tag, typename Y>
        CycleYield<tag, Y const&,
                        Storage<std::vector<rules::element_type_t<Y>>>>
            cycle(rules::Yield<tag, Y> const& yield)
        {
            return {yield._get_()};
        }

        template<template<typename> typename tag, size_t N, typename Y>
        CycleYield<tag, Y const&,
                        Storage<std::array<rules::element_type_t<Y>, N>>>
            cycle(rules::Yield<tag, Y> const& yield)
        {
            return {yield._get_()};
        }
    }

    namespace rules
    {

        template<template<typename> typename tag, typename Y, typename S, typename Sink>
        struct Transfuser<elements::CycleYield<tag, Y, S>, Sink>
        {
            static bool transfuse(elements::CycleYield<tag, Y, S> const& the_yield, Sink& the_sink)
            {
                auto& storage = the_yield.storage;
                auto b = std::begin(the_yield.yield);
                auto e = std::end(the_yield.yield);
                for(; b != e && !storage.full(); ++b)
                {
                    auto&& v = *b;
                    storage.push_back(v);
                    if(!the_sink.consume(FWD(v)))
                        return false;
                }
                for(; b != e; ++b)
                    if(!the_sink.consume(FWD(*b)))
                        return false;
                for(auto&& v: storage)
                    if(!the_sink.consume(FWD(v)))
                        return false;
                return true;
            }
        };

        template<template<typename> typename tag, typename R, typename S, typename Sink>
        struct Transfuser<elements::RangeWrap<tag, elements::CycleIterator<tag, R, S>,
                                          typename elements::CycleIterator<tag, R, S>::Sentinel>,
                          Sink>
        {
            static bool transfuse(elements::RangeWrap<tag, elements::CycleIterator<tag, R, S>,
                                                  typename elements::CycleIterator<tag, R, S>::Sentinel> the_yield,
                                  Sink& the_sink)
            {
                auto i = the_yield.begin();
                auto const& stage = i.stage();
                auto& storage = i.storage();
                if(stage.index() == 1)
                {
                    auto rng1 = std::get<1>(stage);
                    auto b = rng1.interator;
                    auto e = rng1.sentinel;
                    for(; b != e && !storage.full(); ++b)
                    {
                        auto&& v = *b;
                        storage.push_back(v);
                        if(!the_sink.consume(FWD(v)))
                            return false;
                    }
                    for(; b != e; ++b)
                        if(!the_sink.consume(FWD(*b)))
                            return false;
                    for(auto&& v: storage)
                        if(!the_sink.consume(FWD(v)))
                            return false;
                    return true;
                }
                else if(stage.index() == 2)
                    return rules::transfuse(std::get<2>(stage), the_sink);
                else if(stage.index() == 3)
                    return rules::transfuse(std::get<3>(stage), the_sink);
                return true;
            }
        };
    }
}
