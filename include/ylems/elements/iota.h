#pragma once
#include <ylems/rules/abstract.h>
#include <type_traits>
#include <assert.h>

namespace ylems
{
    namespace elements
    {
        template<typename T>
        using difference_type = std::remove_all_extents_t<decltype(std::declval<T>()-std::declval<T>())>;


        template<template<typename> typename tag, typename T, typename D, typename I>
        class Iota: public rules::Yield<tag, Iota<tag, T, D, I>>
        {
            static_assert(std::is_integral_v<I>, "Iota: third template parameter must be integral!");
            T _start;
            D _inc;
            I _count;
        public:
            Iota(Iota&&) = default;
            Iota(Iota const&) = default;

            template<typename TT, typename DD, typename II>
            Iota(TT&& t, DD&& d, II&& i): _start(FWD(t)), _inc(FWD(d)), _count(FWD(i)) {}

            struct Sentinel {};

            I count() const { return _count; }

            class Iterator
            {
                friend class Iota;
                Iterator(Iota const& i): value(i._start), inc(i._inc), count(i._count) {}
                T value;
                D inc;
                I count;
            public:
                T const& operator*() const { return value; }
                Iterator& operator++() { --count; value += inc; return *this; }
                bool operator==(Sentinel) const { return !count; }
                bool operator!=(Sentinel) const { return !!count; }
            };

            Iterator begin() const { return Iterator(*this); }
            Sentinel end() const { return Sentinel{}; }
        };

        template<template<typename> typename tag, typename T, typename D>
        class Iota<tag, T, D, void>: public rules::Yield<tag, Iota<tag, T, D, void>>
        {
            T _start;
            D _inc;
        public:
            Iota(Iota&&) = default;
            Iota(Iota const&) = default;

            template<typename TT, typename DD>
            Iota(TT&& t, DD&& d): _start(FWD(t)), _inc(FWD(d)) {}

            struct Sentinel {};

            class Iterator
            {
                friend class Iota;
                Iterator(Iota const& i): value(i._start), inc(i._inc) {}
                T value;
                D inc;
            public:
                T const& operator*() const { return value; }
                Iterator& operator++() { value += inc; return *this; }
                bool operator==(Sentinel) const { return false; }
                bool operator!=(Sentinel) const { return true; }
            };

            Iterator begin() const { return Iterator(*this); }
            Sentinel end()   const { return Sentinel{}; }
        };

        template<template<typename> typename tag, typename T, typename D, typename I>
        auto iota(T t, D d, I i)
        {
            return Iota<tag, T, D, I>{t, d, i};
        }

        template<template<typename> typename tag, typename T, typename D>
        auto iota(T t, D d)
        {
            return Iota<tag, T, D, void>(t, d);
        }

        template<template<typename> typename tag, typename T, typename I>
        auto linspace(T b, T e, I i)
        {
            return Iota<tag, T, difference_type<T>, I>{b, (e-b)/(i), i+1};
        }

        template<template<typename> typename tag, typename T>
        auto range(T b, T e, T step)
        {
            assert(step != T() && (e-b)/step >= T());
            return Iota<tag, T, difference_type<T>, size_t>{b, step, size_t((e-b)/(step))};
        }

        template<template<typename> typename tag, typename T>
        auto range(T b, T e)
        {
            assert(e >= b);
            return Iota<tag, T, difference_type<T>, size_t>{b, T(1), size_t(e-b)};
        }
    }


}