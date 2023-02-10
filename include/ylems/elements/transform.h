#pragma once
#include <ylems/rules/abstract.h>
#include <assert.h>

#include <ylems/categories/transform.h>

namespace ylems
{
    namespace elements
    {
        template<typename Func, template<typename> typename tag>
        struct TransformWrap: public categories::Transform<TransformWrap<Func, tag>, tag>
        {
            template<typename F>
            TransformWrap(F&& f): transform(FWD(f)) {}

            TransformWrap(TransformWrap&&) = default;
            TransformWrap(TransformWrap const&) = default;

            auto operator()(auto x) const { return transform(x); }

            template<typename S, typename E>
            bool feed(S& sink, E&& e) const
            {
                return sink.eat(transform(FWD(e)));
            }

            Func transform;
        };

        template<template<typename> typename tag, typename F>
        auto transform(F const& f) { return TransformWrap<F const&, tag>{ f}; }

        template<template<typename> typename tag, typename F>
        auto transform(F&& f) { return TransformWrap<F, tag>{FWD(f)}; }

        template<template<typename> typename tag, typename F>
        auto transform(categories::Transform<F, tag>&&)
        {
            assert(false && "Trying to wrap already Transformer in transform!");
        }

        template<template<typename> typename tag, typename F>
        auto transform(categories::Transform<F, tag> const&)
        {
            assert(false && "Trying to wrap already Transformer in transform!");
        }
    }
}