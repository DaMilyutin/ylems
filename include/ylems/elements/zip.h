#pragma once
#include <ylems/rules/abstract.h>
#include <ylems/utilities/tuple_fun.h>

namespace ylems
{
    namespace elements
    {
        template<template<typename> typename tag, typename... T>
        class ZipYield: public rules::Yield<tag, ZipYield<tag, T...>>
        {
            struct beginner { template<typename T> auto operator()(T const& seq) const { return std::begin(seq); } };
            struct ender    { template<typename T> auto operator()(T const& seq) const { return std::end(seq); } };

            template<typename F, typename Tuple, size_t... ints>
            static auto wrap(F f, Tuple const& t, std::index_sequence<ints...>)
            {
                return std::make_tuple(f(std::get<ints>(t))...);
            }

            using TupSeqs = std::tuple<T...>;

            using TupIterators = std::remove_all_extents_t<decltype(wrap(beginner{}, std::declval<TupSeqs>(), std::index_sequence_for<T...>{}))>;
            using TupSentinels = std::remove_all_extents_t<decltype(wrap(ender{}, std::declval<TupSeqs>(), std::index_sequence_for<T...>{}))>;


        public:
            class Iterator;
            class Sentinel
            {
                friend class Iterator;
                TupSentinels sentinels_;
            public:
                using value_type = TupSentinels;
                explicit Sentinel(value_type&& s): sentinels_{std::move(s)} {}
            };

            class Iterator
            {
            public:
                using value_type = std::tuple<decltype(*std::declval<T>().begin())...>;
            private:
                TupIterators iters_;

                template <std::size_t... I>
                auto deref(std::index_sequence<I...>) const
                {
                    return typename Iterator::value_type{*std::get<I>(iters_)...};
                }

                template <std::size_t... I>
                void increment(std::index_sequence<I...>)
                {
                    auto l = {(++std::get<I>(iters_), 0)...};
                }

            public:
                explicit Iterator(TupIterators&& iters): iters_{std::move(iters)} {}

                Iterator& operator++()
                {
                    increment(std::index_sequence_for<T...>{});
                    return *this;
                }

                Iterator operator++(int)
                {
                    auto saved{*this};
                    increment(std::index_sequence_for<T...>{});
                    return saved;
                }

                bool operator!=(const Sentinel& other) const
                {
                    bool ret = true;
                    utilities::tuple_for_each_fn{}(iters_, other.sentinels_,
                               [&ret](auto const& i, auto const& s)
                               {
                                    ret = ret && i != s;
                               });
                    return ret;
                }

                auto operator*() const { return deref(std::index_sequence_for<T...>{}); }
            };

            ZipYield(TupSeqs&& seqs)
                : _seqs(std::forward<TupSeqs>(seqs))
            {}

            Iterator begin() const { return Iterator(wrap(beginner{}, _seqs, std::index_sequence_for<T...>{})); }
            Sentinel end()   const { return Sentinel(wrap(ender{}   , _seqs, std::index_sequence_for<T...>{})); }

        private:
            TupSeqs _seqs;
        };

        template<template<typename> typename tag, typename... T>
        auto zip(rules::Yield<tag, T>&&... seqs)
        {
            return ZipYield<tag, T...>(std::make_tuple(std::move(seqs)._get_()...));
        }

     }
}