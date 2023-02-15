#pragma once
#include <ylems/rules/abstract.h>
#include <limits>
#include <assert.h>

#include <ylems/categories/filter.h>

namespace ylems
{
    namespace elements
    {
        template<typename I, template<typename> typename tag>
        struct DropWrap: public categories::Filter<DropWrap<I, tag>, tag>
        {
            DropWrap(I c): count(c) { assert(count >= I()); }

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
                    {
                        assert(count >= I());
                        while(_it != _end && count-- > I())
                            ++_it;
                    }

                    Iterator& operator++() { ++_it; return *this; }
                    from_t const& operator*() const { return *_it; }
                    bool operator!=(Sentinel) const { return _it != _end; }
                    bool operator==(Sentinel) const { return _it == _end; }

                private:
                    UnderlyingIterator         _it;
                    UnderlyingSentinel         _end;
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
                    return true;
                return (sink._get_()).consume(FWD(e));
            }

            I mutable count;
        };

        template<template<typename> typename tag, typename I>
        auto drop(I count) { return DropWrap<I, tag>{count}; }

        template<template<typename> typename tag, typename I>
        auto drop(DropWrap<I, tag>&&)
        {
            assert(false && "Trying to wrap Drop in Drop!");
        }

        template<template<typename> typename tag, typename I>
        auto drop(DropWrap<I, tag> const&)
        {
            assert(false && "Trying to wrap Drop in Drop!");
        }
    }
}