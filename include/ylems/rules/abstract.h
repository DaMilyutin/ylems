#pragma once
#include <utility>
#include <type_traits>
#include <iterator>

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

        // most generic and can be specialised
        template<typename Y, typename S>
        struct Transfuser
        {
            static bool transfuse(Y const& the_yield, S& the_sink)
            {
                for(auto&& e: the_yield)
                    if(!the_sink.consume(e))
                        return false; // if sink forced to stop
                return true;
            }
        };

        template<typename Y, typename S>
        bool transfuse(Y const& the_yield, S& the_sink)
        {
            return Transfuser<Y, S>::transfuse(the_yield, the_sink);
        }


        template<typename Y>
        struct yield_traits
        {
            using Iterator = std::remove_cvref_t<decltype(std::begin(std::declval<Y>()))>;
            using Sentinel = std::remove_cvref_t<decltype(std::end(std::declval<Y>()))>;
            using Element  = std::remove_cvref_t<decltype(*std::declval<Iterator>())>;
        };

        template<template<typename> typename tag_terminal, typename Y>
        struct yield_traits<Yield<tag_terminal, Y>>
        {
            using Iterator = std::remove_cvref_t<decltype(std::begin(std::declval<Y>()))>;
            using Sentinel = std::remove_cvref_t<decltype(std::end(std::declval<Y>()))>;
            using Element  = std::remove_cvref_t<decltype(*std::declval<Iterator>())>;
        };

        template<typename Y>
        using element_type_t = yield_traits<Y>::Element;

    }
}