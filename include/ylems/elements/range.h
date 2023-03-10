#pragma once
#include <ylems/rules/abstract.h>
#include <ylems/rules/is_a_range.h>
#include <ylems/rules/is_a_yield.h>

namespace ylems
{
    namespace elements
    {
        template<template<typename> typename tag, typename I, typename S>
        struct RangeWrap: public rules::Yield<tag, RangeWrap<tag, I, S>>
        {
            RangeWrap(I b, S e): iterator(b), sentinel(e) {}

            auto begin() const { return iterator; }
            auto end() const   { return sentinel; }

            I iterator;
            S sentinel;
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
    }
}
