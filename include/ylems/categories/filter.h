#pragma once
#include <ylems/rules/abstract.h>
#include <limits>
#include <assert.h>

namespace ylems
{
    namespace categories
    {
        template<typename F, template<typename> typename tag>
        struct Filter: rules::Link<F, tag>
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
                        : _it(std::begin(yield))
                        , _end(std::end(yield))
                        , _select(filter)
                    {
                        next();
                    }

                    from_t const& operator*() const { return _cached; }
                    Iterator& operator++() { if(_it!=_end) ++_it; next(); return *this; }

                    bool operator!=(Sentinel) const { return _it != _end; }
                    bool operator==(Sentinel) const { return _it == _end; }
                private:
                    void next()
                    {
                        while(_it!=_end && !_select(_cached = *_it))
                            ++_it;
                    }

                    UnderlyingIterator         _it;
                    UnderlyingSentinel         _end;
                    F const&                   _select;
                    from_t           mutable   _cached{};
                };

                static auto begin(Y const& y, F const& f) { return Iterator(y, f); }
                static auto end(Y const&, F const&) { return Sentinel{}; }
            };

            template<typename Y> auto begin(Y const& y) const { return YieldDescriptor<Y>::begin(y, this->_get_()); }
            template<typename Y> auto end(Y const& y) const { return YieldDescriptor<Y>::end(y, this->_get_()); }
        };
    }
}