#pragma once
#include <ylems/rules/abstract.h>
#include <assert.h>

namespace ylems
{
    namespace categories
    {
        template<template<typename> typename tag, typename T>
        struct Transform: public rules::Link<tag, T>
        {
            template<typename Y>
            struct YieldDescriptor
            {
                using UnderlyingIterator = std::remove_all_extents_t<decltype(std::begin(std::declval<Y>()))>;
                using UnderlyingSentinel = std::remove_all_extents_t<decltype(std::end(std::declval<Y>()))>;
                using from_t = std::remove_all_extents_t<decltype(*std::begin(std::declval<Y>()))>;

                using Sentinel = std::remove_all_extents_t<decltype(std::end(std::declval<Y>()))>;
                class Iterator
                {
                    using UnderlyingIterator = std::remove_all_extents_t<decltype(std::begin(std::declval<Y>()))>;
                    using from_t = std::remove_cvref_t<decltype(*std::declval<UnderlyingIterator>())>;
                public:
                    Iterator(Y const& yield, T const& transform)
                        : _it(std::begin(yield))
                        , _transform(transform)
                    {}

                    Iterator& operator++() { ++_it; return *this; }
                    auto operator*() const { return _transform(*_it); }
                    bool operator!=(Sentinel const& s) const { return _it != s; }
                    bool operator==(Sentinel const& s) const { return _it == s; }
                private:
                    UnderlyingIterator  _it;
                    T const& _transform;
                };

                static auto begin(Y const& y, T const& l) { return Iterator(y, l); }
                static auto end(Y const& y, T const&) { return Sentinel{std::end(y)}; }
            };

            template<typename S, typename E>
            bool feed(S& sink, E&& e) const
            {
                auto const& transform = this->_get_();
                return sink.consume(transform(FWD(e)));
            }

            template<typename Y> auto begin(Y const& y) const { return YieldDescriptor<Y>::begin(y, this->_get_()); }
            template<typename Y> auto end(Y const& y) const { return YieldDescriptor<Y>::end(y, this->_get_()); }
        };
    }
}