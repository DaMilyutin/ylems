#pragma once
#include <ylems/rules/abstract.h>
#include <limits>
#include <assert.h>

#include <ylems/categories/filter.h>

namespace ylems
{
    namespace elements
    {
        template<template<typename> typename tag, typename I>
        struct DropWrap: public categories::Filter<tag, DropWrap<tag, I>>
        {
            DropWrap(I c): count(c) { assert(count >= I()); }

            template<typename Y>
            struct YieldDescriptor
            {
                using UnderlyingIterator = std::remove_cv_t<decltype(std::begin(std::declval<Y>()))>;
                using UnderlyingSentinel = std::remove_cv_t<decltype(std::end(std::declval<Y>()))>;
                using from_t = decltype(*std::begin(std::declval<Y>()));

                struct Sentinel {};
                class Iterator
                {
                public:
                    Iterator(Y const& yield, I count)
                        : it_(std::begin(yield))
                        , end_(std::end(yield))
                    {
                        assert(count >= I());
                        while(it_ != end_ && count-- > I())
                            *it_, ++it_;
                    }

                    Iterator& operator++() { ++it_; return *this; }
                    from_t operator*() const { return *it_; }
                    bool operator!=(Sentinel) const { return it_ != end_; }
                    bool operator==(Sentinel) const { return it_ == end_; }

                private:
                    UnderlyingIterator         it_;
                    UnderlyingSentinel         end_;
                };

                static auto begin(Y const& y, I i) { return Iterator(y, i); }
                static auto end(Y const&)          { return Sentinel{}; }
            };

            template<typename Y> auto begin(Y const& y) const { return YieldDescriptor<Y>::begin(y, count); }
            template<typename Y> auto end(Y const& y) const { return YieldDescriptor<Y>::end(y); }

            bool operator()(auto const&) const
            {
                if(count > I())
                {
                    --count;
                    return false;
                }
                return true;
            }

            template<typename S, typename E>
            bool feed(S& sink, E&& e) const
            {
                if(count > I())
                    return --count, true;
                return sink.consume(FWD(e));
            }

            I mutable count;
        };

        template<template<typename> typename tag, typename I>
        auto drop(I count) { return DropWrap<tag, I>{count}; }

        template<template<typename> typename tag, typename I>
        auto drop(DropWrap<tag, I>&&)
        {
            assert(false && "Trying to wrap Drop in Drop!");
        }

        template<template<typename> typename tag, typename I>
        auto drop(DropWrap<tag, I> const&)
        {
            assert(false && "Trying to wrap Drop in Drop!");
        }
    }
}