#pragma once
#include <ylems/rules/abstract.h>
#include <ylems/rules/transfuse.h>
#include <ylems/rules/is_a_range.h>
#include <ylems/rules/is_a_yield.h>

namespace ylems
{
    namespace elements
    {
        template<template<typename> typename tag, typename I, typename S>
        struct RangeWrap: public rules::Yield<tag, RangeWrap<tag, I, S>>, public rules::detail::Range<I, S>
        {
            using super = rules::detail::Range<I, S>;

            RangeWrap(I b, S e): super{b, e} {}
            RangeWrap(super&& rhs): super{rhs} {}
            RangeWrap(super const& rhs): super{rhs} {}

            auto begin() const { return super::iterator; }
            auto end() const { return super::sentinel; }
        };

        template<template<typename> typename tag, typename I, typename S>
        RangeWrap<tag, I, S> as_range(I b, S e)
        {
            return {b, e};
        }


        template<template<typename> typename tag, typename R>
        auto as_range(R&& r)
        {
            return as_range<tag>(std::begin(FWD(r)), std::end(FWD(r)));
        }

        template<template<typename> typename tag, typename B, typename E, typename S>
        void advance(RangeWrap<tag, B, E>& rng, ylems::rules::Sink<tag, S>& sink)
        {
            auto& the_sink = sink._get_();
            while(rng.iterator != rng.sentinel && the_sink.consume(*rng.iterator))
                ++rng.iterator;
        }
    }

    namespace rules
    {
        template<template<typename> typename tag, typename B, typename E, typename Sink>
        struct Transfuser<elements::RangeWrap<tag, B, E>, Sink>
        {
            static auto transfuse(elements::RangeWrap<tag, B, E> const& the_yield, Sink& the_sink)
            {
                rules::detail::Range<B, E> rng = the_yield;
                while(rng.iterator != rng.sentinel && the_sink.consume(*rng.iterator))
                    ++rng.iterator;
                return rng;
            }
        };
    }
}
