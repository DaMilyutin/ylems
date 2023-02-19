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
        struct TakeWrap: public categories::Filter<tag, TakeWrap<tag, I>>
        {
            TakeWrap(I c): count(c) { assert(count >= I()); }

            template<typename Y>
            struct YieldDescriptor
            {
                using UnderlyingIterator = std::remove_cv_t<decltype(std::begin(std::declval<Y>()))>;
                using UnderlyingSentinel = std::remove_cv_t<decltype(std::end(std::declval<Y>()))>;
                using from_t = std::remove_cvref_t<decltype(*std::begin(std::declval<Y>()))>;

                struct Sentinel {};
                class Iterator
                {
                public:
                    Iterator(Y const& yield, I count)
                        : _it(std::begin(yield))
                        , _end(std::end(yield))
                        , _count(count)
                    {
                        assert(count >= I());
                    }

                    Iterator& operator++() { ++_it; --_count; return *this; }
                    from_t const& operator*() const { return *_it; }
                    bool operator!=(Sentinel) const { return _it != _end && _count > I(); }
                    bool operator==(Sentinel) const { return _it == _end || _count == I(); }

                private:
                    UnderlyingIterator         _it;
                    UnderlyingSentinel         _end;
                    I                          _count;
                };

                static auto begin(Y const& y, I i) { return Iterator(y, i); }
                static auto end(Y const&)          { return Sentinel{}; }
            };

            template<typename Y> auto begin(Y const& y) const { return YieldDescriptor<Y>::begin(y, count); }
            template<typename Y> auto end(Y const& y) const { return YieldDescriptor<Y>::end(y); }

            bool operator()(auto const&) const
            {
                return count-- > I();
            }

            template<typename S, typename E>
            bool feed(S& sink, E&& e) const
            {
                if(count-- > I())
                    return sink.consume(FWD(e));
                return true;
            }

            I mutable count;
        };

        template<template<typename> typename tag, typename I>
        auto take(I count) { return TakeWrap<I, tag>{count}; }

        template<template<typename> typename tag, typename I>
        auto take(TakeWrap<tag, I>&&)
        {
            assert(false && "Trying to wrap Take in Take!");
        }

        template<template<typename> typename tag, typename I>
        auto take(TakeWrap<tag, I> const&)
        {
            assert(false && "Trying to wrap Take in Take!");
        }
    }
}