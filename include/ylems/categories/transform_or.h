#pragma once
#include <ylems/rules/abstract.h>
#include <optional>
#include <iterator>

namespace ylems
{
    namespace categories
    {
        template<template<typename> typename tag, typename E>
        struct TransformOr: public rules::Link<tag, E>
        {
            template<typename Y>
            struct YieldDescriptor
            {
                using UnderlyingIterator = std::remove_cv_t<decltype(std::begin(std::declval<Y>()))>;
                using UnderlyingSentinel = std::remove_cv_t<decltype(std::end(std::declval<Y>()))>;

                using from_t = std::remove_cvref_t<decltype(*std::begin(std::declval<Y>()))>;
                using to_t = std::remove_cvref_t<decltype(std::declval<E>()(std::declval<from_t>()))>;
                using unwrapped_t = std::remove_cvref_t<decltype(*std::declval<to_t>())>;

                struct Sentinel {};

                class Iterator
                {
                public:
                    Iterator(Y const& y, E const& l)
                        : it_(std::begin(y))
                        , end_(std::end(y))
                        , transform_(l)
                    {
                        next();
                    }

                    unwrapped_t const& operator*() const { return *cached_; }
                    Iterator& operator++() { if(it_!=end_) ++it_; next(); return *this; }

                    bool operator!=(Sentinel) const { return it_ != end_; }
                    bool operator==(Sentinel) const { return it_ == end_; }
                private:
                    void next()
                    {
                        while(it_!=end_ && !(cached_ = transform_(*it_)))
                            ++it_;
                    }

                    UnderlyingIterator           it_;
                    UnderlyingSentinel           end_;
                    E                  const&    transform_;
                    to_t              mutable    cached_{};
                };

                static auto begin(Y const& y, E const& l) { return Iterator(y, l); }
                static auto end(Y const&, E const&) { return Sentinel{}; }
            };

            template<typename S, typename E>
            bool feed(S& sink, E&& e) const
            {
                auto const& transform = this->_get_();
                auto result = transform(FWD(e));
                if(result)
                    return sink.consume(*result);
                return true;
            }

            template<typename Y> auto begin(Y const& y) const { return YieldDescriptor<Y>::begin(y, this->_get_()); }
            template<typename Y> auto end(Y const& y) const { return YieldDescriptor<Y>::end(y, this->_get_()); }
        };
    }
}