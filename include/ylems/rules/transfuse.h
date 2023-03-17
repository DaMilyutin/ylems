#pragma once
#include <iterator>

namespace ylems
{
    namespace rules
    {
        namespace detail
        {
            template<typename I, typename S>
            struct Range
            {
                I iterator;
                S sentinel;
            };

            template<typename I, typename S>
            auto make_range(I i, S s)
            {
                return Range<I,S>{i, s};
            }
        }

        // most generic and can be specialized
        template<typename Y, typename S>
        struct Transfuser
        {
            static auto transfuse(Y const& the_yield, S& the_sink)
            {
                auto rng = detail::make_range(std::begin(the_yield), std::end(the_yield));
                while(rng.iterator != rng.sentinel && the_sink.consume(*rng.iterator))
                    ++rng.iterator;
                return rng;
            }
        };

        template<typename Y, typename S>
        auto transfuse(Y const& the_yield, S& the_sink)
        {
            return Transfuser<Y, S>::transfuse(the_yield, the_sink);
        }
    }
}