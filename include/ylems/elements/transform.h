#pragma once
#include <ylems/rules/abstract.h>
#include <assert.h>

#include <ylems/categories/transform.h>

namespace ylems
{
    namespace elements
    {
        template<template<typename> typename tag, typename Func>
        struct TransformWrap: public categories::Transform<tag, TransformWrap<tag, Func>>
        {
            template<typename F>
            TransformWrap(F&& f): transform(FWD(f)) {}

            TransformWrap(TransformWrap&&) = default;
            TransformWrap(TransformWrap const&) = default;

            auto operator()(auto x) const { return transform(x); }

            template<typename S, typename E>
            bool feed(S& sink, E&& e) const
            {
                return sink.consume(transform(FWD(e)));
            }

            Func transform;
        };

        template<template<typename> typename tag, typename F>
        auto transform(F const& f) { return TransformWrap<tag, F const&>{ f}; }

        template<template<typename> typename tag, typename F>
        auto transform(F&& f) { return TransformWrap<tag, F>{FWD(f)}; }

        template<template<typename> typename tag, typename F>
        auto transform(categories::Transform<tag, F>&&)
        {
            assert(false && "Trying to wrap already Transformer in transform!");
        }

        template<template<typename> typename tag, typename F>
        auto transform(categories::Transform<tag, F> const&)
        {
            assert(false && "Trying to wrap already Transformer in transform!");
        }
    }
}