#pragma once
#include <ylems/rules/abstract.h>
#include <limits>
#include <assert.h>

#include <ylems/categories/filter.h>

namespace ylems
{
    namespace elements
    {
        template<template<typename> typename tag, typename C>
        struct TeeWrap: public rules::Link<tag, TeeWrap<tag, C>>
        {
            template<typename X>
            TeeWrap(X&& x): callback(FWD(x)) {}

            template<typename Y>
            struct YieldDescriptor
            {
                using UnderlyingIterator = std::remove_cv_t<decltype(std::begin(std::declval<Y>()))>;
                using UnderlyingSentinel = std::remove_cv_t<decltype(std::end(std::declval<Y>()))>;
                using from_t = std::remove_cvref_t<decltype(*std::begin(std::declval<Y>()))>;

                using Sentinel = UnderlyingSentinel;

                class Iterator
                {
                public:
                    Iterator(Y const& yield, C& cb)
                        : it_(std::begin(yield))
                        , callback_(cb)
                    {}

                    Iterator& operator++() { ++it_; return *this; }
                    auto operator*() const { auto&& v = *it_; callback_(v); return FWD(v); }
                    bool operator!=(Sentinel s) const { return it_ != s; }
                    bool operator==(Sentinel s) const { return it_ == s; }

                private:
                    UnderlyingIterator         it_;
                    C&                         callback_;
                };

                static auto begin(Y const& y, C& cb) { return Iterator(y, cb); }
                static auto end(Y const& y) { return end(y); }
            };

            template<typename Y> auto begin(Y const& y) const { return YieldDescriptor<Y>::begin(y, callback); }
            template<typename Y> auto end(Y const& y) const { return YieldDescriptor<Y>::end(y); }

            template<typename S, typename E>
            bool feed(S& sink, E&& e) const
            {
                callback(e);
                return sink.consume(FWD(e));
            }

            C callback;
        };

        template<template<typename> typename tag, typename C>
        auto tee(C&& cb) { return TeeWrap<tag, C>{FWD(cb)}; }

        template<template<typename> typename tag, typename C>
        auto tee(C& cb) { return TeeWrap<tag, C&>{cb}; }


    }
}