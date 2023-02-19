#pragma once
#include <utility>
#include <type_traits>

#define FWD(a) std::forward<decltype(a)>(a)

namespace ylems
{
    namespace rules
    {
        template<typename E>
        struct _terminal_
        {
            E& _get_()& { return static_cast<E&>(*this); }
            E const& _get_() const& { return static_cast<E const&>(*this); }
            E&& _get_()&& { return static_cast<E&&>(*this); }
        };

        //TODO: concept/detect for Yield

        template<template<typename> typename tag_terminal, typename E>
        struct Yield: public tag_terminal<E>
        {};

        template<template<typename> typename tag_terminal, typename E>
        struct Link: public tag_terminal<E>
        {
            //for specific Link category(ex.Filter, Transform) one must provide methods
            //    template<typename Y> auto begin(Y const& y) const
            //    template<typename Y> auto end(Y const& y) const
            // for connection to Yield
            // and method
            //    template<typename Sink, typename Elem> bool feed(Sink const&, Elem&&)
            // for connection to Sink
        };

        template<template<typename> typename tag_terminal, typename E>
        struct Sink: public tag_terminal<E>
        {};
    }
}