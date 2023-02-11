#pragma once
#include <ylems/rules/abstract.h>
#include <optional>

#include <ylems/categories/transform_or.h>

namespace ylems
{
    namespace elements
    {
        template<typename Func, template<typename> typename tag>
        struct TransformOrWrap: public categories::TransformOr<TransformOrWrap<Func, tag>, tag>
        {
            template<typename F>
            TransformOrWrap(F&& f): transform(FWD(f)) {}

            auto operator()(auto&& x) const { return transform(FWD(x)); }

            template<typename S, typename E>
            bool feed(S& sink, E&& e) const
            {
                auto res = transform(FWD(e));
                if(res) // nullable type assumed
                    return sink.consume(*res);
                return true;
            }
            Func  transform;
        };

        template<typename Sel, typename Trans, template<typename> typename tag>
        struct TransformOrWrap2: public categories::TransformOr<TransformOrWrap2<Sel, Trans, tag>, tag>
        {
            template<typename F, typename G>
            TransformOrWrap2(F&& f, G&& g): select(FWD(f)), transform(FWD(g)) {}

            template<typename S, typename E>
            bool feed(S& sink, E&& e) const
            {
                if(select(e))
                    return sink.consume(transform(FWD(e)));
                return true;
            }

            auto operator()(auto&& x) const { return select(x) ? std::make_optional(transform(FWD(x))) : std::nullopt; }
            Sel   select;
            Trans transform;
        };

        template<template<typename> typename tag, typename F>
        auto transform_or(F const& f) { return TransformOrWrap<F const&, tag>{ f}; }

        template<template<typename> typename tag, typename F>
        auto transform_or(F&& f) { return TransformOrWrap<F, tag>{std::move(f)}; }

        template<template<typename> typename tag, typename F, typename G>
        auto transform_or(F&& f, G&& g) { return TransformOrWrap2<F, G, tag>{FWD(f), FWD(g)}; }

        template<template<typename> typename tag, typename F>
        auto transform_or(categories::TransformOr<F, tag>&&)
        {
            assert(false && "Trying to wrap already Transformer in transform!");
        }

        template<template<typename> typename tag, typename F>
        auto transform_or(categories::TransformOr<F, tag> const&)
        {
            assert(false && "Trying to wrap already Transformer in transform!");
        }
    }
}