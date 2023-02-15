#pragma once
#include <ylems/rules/abstract.h>
#include <ylems/rules/is_a_range.h>

namespace ylems
{
    namespace elements
    {
        template<typename Range, template<typename> typename tag>
        struct YieldWrap: public rules::Yield<YieldWrap<Range, tag>, tag>
        {
            template<typename R>
            YieldWrap(R&& r): range(std::forward<R>(r)) {}

            auto begin() const { return std::begin(range); }
            auto end() const   { return std::end(range);   }

            Range range;
        };

        template<template<typename> typename tag, typename R>
        auto yield(R const& r) { return YieldWrap<R const&, tag>{r}; }

        template<template<typename> typename tag, typename R>
        auto yield(R&& r) { return YieldWrap<R, tag>{FWD(r)}; }

        template<template<typename> typename tag, typename R>
        void yield(rules::Yield<R, tag>&&) = delete;  // forbid

        template<template<typename> typename tag, typename R>
        void yield(rules::Yield<R, tag> const&) = delete; // forbid
    }
}

#define YLEMS_MELD_RANGE_OPERATION(tag, OP)                                                            \
template<typename L, typename R, ylems::concepts::IsARange<L> = true> auto OP(L&& l, tag<R>&& r)       \
{ return ylems::rules::meld_tag<tag>(ylems::elements::yield<tag>(FWD(l)), FWD(r)); }                        \
template<typename L, typename R, ylems::concepts::IsARange<L> = true> auto OP(L const& l, tag<R>&& r)  \
{ return ylems::rules::meld_tag<tag>(ylems::elements::yield<tag>(l), FWD(r)); }