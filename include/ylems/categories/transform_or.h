#pragma once
#include <ylems/rules/abstract.h>
#include <optional>

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
                        : _it(std::begin(y))
                        , _end(std::end(y))
                        , _transform(l)
                    {
                        next();
                    }

                    unwrapped_t const& operator*() const { return *_cached; }
                    Iterator& operator++() { if(_it!=_end) ++_it; next(); return *this; }

                    bool operator!=(Sentinel) const { return _it != _end; }
                    bool operator==(Sentinel) const { return _it == _end; }
                private:
                    void next()
                    {
                        while(_it!=_end && !(_cached = _transform(*_it)))
                            ++_it;
                    }

                    UnderlyingIterator           _it;
                    UnderlyingSentinel           _end;
                    E                  const&    _transform;
                    to_t              mutable    _cached{};
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