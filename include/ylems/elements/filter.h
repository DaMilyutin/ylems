#pragma once
#include <ylems/rules/abstract.h>
#include <limits>
#include <assert.h>

#include <ylems/categories/filter.h>

namespace ylems
{
    namespace elements
    {
        template<template<typename> typename tag, typename Func>
        struct FilterWrap: public categories::Filter<tag, FilterWrap<tag, Func>>
        {
            template<typename F>
            FilterWrap(F&& f): select(std::forward<F>(f)) {}

            bool operator()(auto const& x) const { return select(x); }

            template<typename S, typename E>
            bool feed(S& sink, E&& e) const
            {
                if(!select(e))
                    return false;
                return sink.consume(FWD(e));
            }

            Func select;
        };

        template<template<typename> typename tag, typename F>
        auto filter(F const& f) { return FilterWrap<F const&, tag>{f}; }

        template<template<typename> typename tag, typename F>
        auto filter(F&& f) { return FilterWrap<F, tag>{ std::move(f)}; }

        template<template<typename> typename tag, typename F>
        auto filter(categories::Filter<tag, F>&&)
        {
            assert(false && "Trying to wrap Filter in filter!");
        }

        template<template<typename> typename tag, typename F>
        auto filter(categories::Filter<tag, F> const&)
        {
            assert(false && "Trying to wrap Filter in filter!");
        }
    }
}