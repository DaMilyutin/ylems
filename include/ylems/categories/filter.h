#pragma once
#include <ylems/rules/abstract.h>
#include <limits>
#include <assert.h>
#include <iterator>

namespace ylems
{
    namespace categories
    {
        template<template<typename> typename tag, typename F>
        struct Filter: rules::Link<tag, F>
        {
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
                    Iterator(Y const& yield, F const& filter)
                        : it_(std::begin(yield))
                        , end_(std::end(yield))
                        , select_(filter)
                    {
                        next();
                    }

                    from_t const& operator*() const { return cached_; }
                    Iterator& operator++() { if(it_!=end_) ++it_; next(); return *this; }

                    bool operator!=(Sentinel) const { return it_ != end_; }
                    bool operator==(Sentinel) const { return it_ == end_; }
                private:
                    void next()
                    {
                        while(it_!=end_ && !select_(cached_ = *it_))
                            ++it_;
                    }

                    UnderlyingIterator         it_;
                    UnderlyingSentinel         end_;
                    F const&                   select_;
                    from_t           mutable   cached_{};
                };

                static auto begin(Y const& y, F const& f) { return Iterator(y, f); }
                static auto end(Y const&, F const&) { return Sentinel{}; }
            };

            template<typename S, typename E>
            bool feed(S& sink, E&& e) const
            {
                auto const& filter = this->_get_();
                if(filter(e))
                    return sink.consume(FWD(e));
                return true;
            }

            template<typename Y> auto begin(Y const& y) const { return YieldDescriptor<Y>::begin(y, this->_get_()); }
            template<typename Y> auto end(Y const& y) const { return YieldDescriptor<Y>::end(y, this->_get_()); }
        };
    }
}