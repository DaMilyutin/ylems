#pragma once
#include <ylems/rules/abstract.h>

namespace ylems
{
    namespace rules
    {
        template<typename Y, typename L, template<typename> typename tag>
        struct YieldLink: public Yield<YieldLink<Y, L, tag>, tag>
        {
            template<typename TY, typename TL>
            YieldLink(TY&& y, TL&& l): yield(FWD(y)), link(FWD(l)) {}

            auto begin() const { return link.begin(yield); }
            auto end()   const { return link.end(yield); }

            Y yield;
            L link;
        };

        template<typename L, typename S, template<typename> typename tag>
        struct LinkSink: public Sink<LinkSink<L, S, tag>, tag>
        {
            template<typename TL, typename TS>
            LinkSink(TL&& l, TS&& s): link(FWD(l)), sink(FWD(s)) {}

            template<typename E>
            bool operator()(E&& e) { return link.feed(sink, e); }

            L link;
            S sink;
        };

        template<typename L1, typename L2, template<typename> typename tag>
        struct LinkLink: public Link<LinkLink<L1, L2, tag>, tag>
        {
            template<typename T1, typename T2>
            LinkLink(T1&& l1, T2&& l2): link1(FWD(l1)), link2(FWD(l2)) {}

            template<typename S, typename E>
            bool feed(S& sink, E&& e)
            {
                LinkSink<L2&, S&> sink2(link2, sink);
                return link1.feed(sink2, FWD(e));
            }

            L1 link1;
            L2 link2;
        };

        // binary operations
        template<template<typename> typename tag, typename Y, typename L>
        YieldLink<Y, L, tag> meld(Yield<Y, tag>&& y, Link<L, tag>&& l)
        {
            return {FWD(y)._get_(), FWD(l)._get_()};
        }

        template<template<typename> typename tag, typename Y, typename L>
        YieldLink<Y const&, L, tag> meld(Yield<Y, tag> const& y, Link<L, tag>&& l)
        {
            return {y._get_(), FWD(l)._get_()};
        }

        template<template<typename> typename tag, typename Y, typename L>
        YieldLink<Y, L const&, tag> meld(Yield<Y, tag>&& y, Link<L, tag> const& l)
        {
            return {FWD(y)._get_(), l._get_()};
        }

        template<template<typename> typename tag, typename Y, typename L>
        YieldLink<Y const&, L const&, tag> meld(Yield<Y, tag> const& y, Link<L, tag> const& l)
        {
            return {y._get_(), l._get_()};
        }

        template<template<typename> typename tag, typename L1, typename L2>
        LinkLink<L1, L2, tag> meld(Link<L1, tag>&& l1, Link<L2, tag>&& l2)
        {
            return {FWD(l1)._get_(), FWD(l2)._get_()};
        }

        template<template<typename> typename tag, typename L, typename S>
        LinkSink<L, S, tag> meld(Link<L, tag>&& l, Sink<S, tag>&& s)
        {
            return {FWD(l)._get_(), FWD(s)._get_()};
        }

        // Yield + Sink => system closed and ready to run

        template<template<typename> typename tag, typename Y, typename S>
        bool meld(Yield<Y, tag> const& yield, Sink<S, tag>& sink)
        {
            auto& the_sink = sink._get_();
            for(auto&& e: yield._get_())
                if(!the_sink.consume(e))
                    return false; // if sink forced to stop
            return true;
        }

        template<template<typename> typename tag, typename Y, typename S>
        bool meld(Yield<Y, tag>&& yield, Sink<S, tag>& sink) { return meld(yield._get_(), sink._get_()); }

        template<template<typename> typename tag, typename Y, typename S>
        bool meld(Yield<Y, tag> const& yield, Sink<S, tag>&& sink) { return meld(yield._get_(), sink._get_()); }

        template<template<typename> typename tag, typename Y, typename S>
        bool meld(Yield<Y, tag>&& yield, Sink<S, tag>&& sink) { return meld(yield._get_(), sink._get_()); }


        // Yield + Sink: We prefer to keep Yield simple
        template<template<typename> typename tag, typename Y, typename L, typename S>
        bool meld(YieldLink<Y, L, tag>&& yl, Sink<S, tag>&& sink)
        {
            return meld(FWD(yl).yield, meld(FWD(yl).link, FWD(sink)));
        }

        // reorganize pipeline around sink
        template<template<typename> typename tag, typename L1, typename L2, typename S>
        LinkSink<L1, LinkSink<L2, S, tag>, tag> meld(LinkLink<L1, L2, tag>&& ll, Sink<S, tag>&& s)
        {
            return meld(FWD(ll).link1, meld(FWD(ll).link2, FWD(s)));
        }

        // reorganize pipeline around yield
        template<template<typename> typename tag, typename Y, typename L1, typename L2>
        YieldLink<YieldLink<Y, L1, tag>, L2, tag> meld(Yield<Y, tag>&& y, LinkLink<L1, L2, tag>&& ll)
        {
            return meld(meld(FWD(y), FWD(ll).link1), FWD(ll).link2);
        }

        // helpers to kick-in ADL and static polymorphism
        template<template<typename> typename tag, typename X, typename Y>
        auto meld_tag(tag<X>&& x, tag<Y>&& y)
        {
            return meld<tag>(FWD(x)._get_(), FWD(y)._get_());
        }

        template<template<typename> typename tag, typename X, typename Y>
        auto meld_tag(tag<X> const& x, tag<Y>&& y)
        {
            return meld<tag>(x._get_(), FWD(y)._get_());
        }

        template<template<typename> typename tag, typename X, typename Y>
        auto meld_tag(tag<X>&& x, tag<Y> const& y)
        {
            return meld<tag>(FWD(x)._get_(), y._get_());
        }

        template<template<typename> typename tag, typename X, typename Y>
        auto meld_tag(tag<X> const& x, tag<Y> const& y)
        {
            return meld<tag>(x._get_(), y._get_());
        }

        // last two for Sink actually
        template<template<typename> typename tag, typename X, typename Y>
        auto meld_tag(tag<X>&& x, tag<Y>& y) // helper to kick-in ADL and static polymorphism
        {
            return meld<tag>(FWD(x)._get_(), y._get_());
        }

        template<template<typename> typename tag, typename X, typename Y>
        auto meld_tag(tag<X> const& x, tag<Y>& y)
        {
            return meld<tag>(x._get_(), y._get_());
        }

    }
}

#define YLEMS_MELD_OPERATION(tag, OP) \
template<typename L, typename R> auto OP(tag<L>&&      l, tag<R>&&      r) { return ylems::rules::meld_tag(FWD(l), FWD(r)); } \
template<typename L, typename R> auto OP(tag<L> const& l, tag<R>&&      r) { return ylems::rules::meld_tag(l     , FWD(r)); } \
template<typename L, typename R> auto OP(tag<L>&&      l, tag<R> const& r) { return ylems::rules::meld_tag(FWD(l), r     ); } \
template<typename L, typename R> auto OP(tag<L> const& l, tag<R> const& r) { return ylems::rules::meld_tag(l     , r     ); } \
template<typename L, typename R> auto OP(tag<L>&&      l, tag<R>&       r) { return ylems::rules::meld_tag(FWD(l), r     ); } \
template<typename L, typename R> auto OP(tag<L> const& l, tag<R>&       r) { return ylems::rules::meld_tag(l     , r     ); }