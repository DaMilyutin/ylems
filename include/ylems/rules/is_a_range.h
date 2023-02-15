#pragma once

#include <utility>
#include <ylems/utilities/detect.h>

namespace ylems
{
    namespace concepts
    {
        namespace impl
        {
            // definition of range

            namespace adl
            {
                using std::begin;
                using std::end;
                template<typename T>
                using begin_expression = decltype(begin(std::declval<T&>()));
                template<typename T>
                using end_expression = decltype(end(std::declval<T&>()));
            }

            template<typename Range>
            constexpr bool range_expression_detected = utilities::detail::is_detected<adl::begin_expression, Range>
                                                    && utilities::detail::is_detected<adl::end_expression, Range>;

            template<typename Range>
            using IsARange = std::enable_if_t<range_expression_detected<Range>, bool>;
        }

        template<typename Range>
        using IsARange = impl::IsARange<std::remove_reference_t<Range>>;
    }
}