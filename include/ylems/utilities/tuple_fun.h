#pragma once
#include <tuple>

namespace ylems
{
    namespace utilities
    {
        template<typename Tup>
        using tuple_indices_t = std::make_index_sequence<
                                    std::tuple_size<typename std::remove_reference<Tup>::type>::value>;

        struct tuple_for_each_fn
        {
        private:
            template<typename Tup, typename Fun, std::size_t... Is>
            static constexpr void impl(Tup&& tup, Fun& fun, std::index_sequence<Is...>)
            {
                (void)std::initializer_list<int>{
                    ((void)fun(std::get<Is>(static_cast<Tup&&>(tup))), 42)...};
            }

            template<typename Tup1, typename Tup2, typename Fun, std::size_t... Is>
            static constexpr void impl(Tup1&& tup1, Tup2&& tup2, Fun& fun, std::index_sequence<Is...>)
            {
                (void)std::initializer_list<int>{
                    ((void)fun(std::get<Is>(static_cast<Tup1&&>(tup1)),
                               std::get<Is>(static_cast<Tup2&&>(tup2))), 42)...};
            }
        public:
            template<typename Tup, typename Fun>
            constexpr Fun operator()(Tup&& tup, Fun fun) const
            {
                return tuple_for_each_fn::impl(
                    static_cast<Tup&&>(tup), fun, tuple_indices_t<Tup>{}),
                    fun;
            }

            template<typename Tup1, typename Tup2, typename Fun>
            constexpr Fun operator()(Tup1&& tup1, Tup2&& tup2, Fun fun) const
            {
                return tuple_for_each_fn::impl(
                    static_cast<Tup1&&>(tup1), static_cast<Tup2&&>(tup2), fun, tuple_indices_t<Tup1>{}),
                    fun;
            }
        };
    }
}