#pragma once

#include <utility>
#include <ylems/rules/abstract.h>

namespace ylems
{
    namespace concepts
    {
        template<template<typename> typename tag, typename Range>
        using IsAYield = std::enable_if_t<std::is_base_of_v<ylems::rules::Yield<tag, std::remove_cvref_t<Range>>, std::remove_cvref_t<Range>>, bool>;

        template<template<typename> typename tag, typename Range>
        using NotAYield = std::enable_if_t<!std::is_base_of_v<ylems::rules::Yield<tag, std::remove_cvref_t<Range>>, std::remove_cvref_t<Range>>, bool>;
    }
}