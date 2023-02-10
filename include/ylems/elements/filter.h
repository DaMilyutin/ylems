#pragma once
#include <ylems/rules/abstract.h>
#include <limits>
#include <assert.h>

#include <ylems/categories/filter.h>

namespace ylems
{
    namespace elements
    {
        template<typename Func, template<typename> typename tag>
        struct FilterWrap: public categories::Filter<FilterWrap<Func, tag>, tag>
        {
            template<typename F>
            FilterWrap(F&& f): select(std::forward<F>(f)) {}

            bool operator()(auto const& x) const { return select(x); }

            template<typename S, typename E>
            bool feed(S& sink, E&& e) const
            {
                if(!select(e))
                    return false;
                return (sink._get_()) << FWD(e);
            }

            Func select;
        };

        template<template<typename> typename tag, typename F>
        auto filter(F const& f) { return FilterWrap<F const&, tag>{f}; }

        template<template<typename> typename tag, typename F>
        auto filter(F&& f) { return FilterWrap<F, tag>{ std::move(f)}; }

        template<template<typename> typename tag, typename F>
        auto filter(categories::Filter<F, tag>&&)
        {
            assert(false && "Trying to wrap Filter in filter!");
        }

        template<template<typename> typename tag, typename F>
        auto filter(categories::Filter<F, tag> const&)
        {
            assert(false && "Trying to wrap Filter in filter!");
        }
    }
}