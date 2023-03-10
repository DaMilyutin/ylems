#pragma once
#include <ylems/rules/abstract.h>
#include <limits>
#include <assert.h>

#include <ylems/categories/transform.h>

#include <ylems/utilities/CycleBuffer.h>

namespace ylems
{
    namespace elements
    {
        template<template<typename> typename tag, typename Storage>
        struct MemoizeWrap: public categories::Transform<tag, MemoizeWrap<tag, Storage>>
        {
            MemoizeWrap() = default;

            template<typename T>
            MemoizeWrap(T&& s): buffer(FWD(s)) {}

            auto const& operator()(auto&& e) const
            {
                buffer.push_back(FWD(e));
                return buffer;
            }

            template<typename S, typename E>
            bool feed(S& sink, E&& e) const
            {
                buffer.push_back(FWD(e));
                return sink.consume(buffer);
            }

            Storage mutable buffer;
        };

        template<template<typename> typename tag, typename S>
        auto memoize(S& s) { return MemoizeWrap<tag, S&>{s}; }

        template<template<typename> typename tag, typename S>
        auto memoize(S&& s) { return MemoizeWrap<tag, S>{FWD(s)}; }

        template<template<typename> typename tag, typename T, size_t N>
        auto memoize() { return MemoizeWrap<tag, CycleBuffer<T, N>>{}; }

        template<template<typename> typename tag, typename S>
        auto memoize(categories::Transform<tag, S>&&)
        {
            assert(false && "Trying to wrap already Transformer in memoize!");
        }

        template<template<typename> typename tag, typename S>
        auto memoize(categories::Transform<tag, S> const&)
        {
            assert(false && "Trying to wrap already Transformer in memoize!");
        }

    }
}