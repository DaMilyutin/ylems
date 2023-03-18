#pragma once
#include <ylems/rules/abstract.h>
#include <limits>
#include <assert.h>

namespace ylems
{
    namespace elements
    {
        template<template<typename> typename tag, typename C>
        struct PushBack: public rules::Sink<tag, PushBack<tag, C>>
        {
            PushBack(C& c): container(c) {}

            bool consume(auto&& p)
            {
                container.push_back(FWD(p));
                return true;
            }

            C& container;
        };

        template<template<typename> typename tag, typename C>
        auto push_back(C& c) { return PushBack<tag, C>(c); }
    }
}