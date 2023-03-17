#pragma once
#include <ylems/rules/abstract.h>

namespace ylems
{
    namespace rules
    {
        template<template<typename> typename tag, typename Y, typename L>
        struct YieldLink: public Yield<tag, YieldLink<tag, Y, L>>
        {
            template<typename TY, typename TL>
            YieldLink(TY&& y, TL&& l)
                : yield(FWD(y))
                , link(FWD(l))
            {}

            YieldLink<tag, Y const&, L const&> as_const() const
            {
                return {yield, link};
            }

            auto begin() const { return link.begin(yield); }
            auto end()   const { return link.end(yield); }

            Y yield;
            L link;
        };

        template<template<typename> typename tag, typename L, typename S>
        struct LinkSink: public Sink<tag, LinkSink<tag, L, S>>
        {
            template<typename TL, typename TS>
            LinkSink(TL&& l, TS&& s)
                : link(FWD(l))
                , sink(FWD(s))
            {}

            template<typename E>
            bool consume(E&& e) { return link.feed(sink, FWD(e)); }

            L link;
            S sink;
        };

        template<template<typename> typename tag, typename L1, typename L2>
        struct LinkLink: public Link<tag, LinkLink<tag, L1, L2>>
        {
            template<typename T1, typename T2>
            LinkLink(T1&& l1, T2&& l2)
                : link1(FWD(l1))
                , link2(FWD(l2))
            {}

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
        YieldLink<tag, Y, L> meld(Yield<tag, Y>&& y, Link<tag, L>&& l)
        {
            return {FWD(y)._get_(), FWD(l)._get_()};
        }

        template<template<typename> typename tag, typename Y, typename L>
        YieldLink<tag, Y const&, L> meld(Yield<tag, Y> const& y, Link<tag, L>&& l)
        {
            return {y._get_(), FWD(l)._get_()};
        }

        template<template<typename> typename tag, typename Y, typename L>
        YieldLink<tag, Y, L const&> meld(Yield<tag, Y>&& y, Link<tag, L> const& l)
        {
            return {FWD(y)._get_(), l._get_()};
        }

        template<template<typename> typename tag, typename Y, typename L>
        YieldLink<tag, Y const&, L const&> meld(Yield<tag, Y> const& y, Link<tag, L> const& l)
        {
            return {y._get_(), l._get_()};
        }

        template<template<typename> typename tag, typename L1, typename L2>
        LinkLink<tag, L1, L2> meld(Link<tag, L1>&& l1, Link<tag, L2>&& l2)
        {
            return {FWD(l1)._get_(), FWD(l2)._get_()};
        }

        template<template<typename> typename tag, typename L, typename S>
        LinkSink<tag, L, S> meld(Link<tag, L>&& l, Sink<tag, S>&& s)
        {
            return {FWD(l)._get_(), FWD(s)._get_()};
        }

        template<template<typename> typename tag, typename L, typename S>
        LinkSink<tag, L, S> meld(Link<tag, L> const& l, Sink<tag, S>&& s)
        {
            return {l._get_(), FWD(s)._get_()};
        }

        template<template<typename> typename tag, typename L, typename S>
        LinkSink<tag, L, S&> meld(Link<tag, L>&& l, Sink<tag, S>& s)
        {
            return {FWD(l)._get_(), s._get_()};
        }

        template<template<typename> typename tag, typename L, typename S>
        LinkSink<tag, L, S&> meld(Link<tag, L> const& l, Sink<tag, S>& s)
        {
            return {l._get_(), s._get_()};
        }


        // Yield + Sink => system closed and ready to run

        template<template<typename> typename tag, typename Y, typename S>
        auto meld(Yield<tag, Y> const& yield, Sink<tag, S>& sink)
        {
            auto& the_sink = sink._get_();
            auto const& the_yield = yield._get_();
            return transfuse(the_yield, the_sink);
        }

        template<template<typename> typename tag, typename Y, typename S>
        auto meld(Yield<tag, Y>&& yield, Sink<tag, S>& sink) { return meld(yield._get_(), sink._get_()); }

        template<template<typename> typename tag, typename Y, typename S>
        auto meld(Yield<tag, Y> const& yield, Sink<tag, S>&& sink) { return meld(yield._get_(), sink._get_()); }

        template<template<typename> typename tag, typename Y, typename S>
        auto meld(Yield<tag, Y>&& yield, Sink<tag, S>&& sink) { return meld(yield._get_(), sink._get_()); }


        // Yield + Sink: We prefer to keep Yield simple
        template<template<typename> typename tag, typename Y, typename L, typename S>
        auto meld(YieldLink<tag, Y, L>&& yl, Sink<tag, S>&& sink)
        {
            return meld(FWD(yl).yield, meld(FWD(yl).link, FWD(sink)));
        }

        template<template<typename> typename tag, typename Y, typename L, typename S>
        auto meld(YieldLink<tag, Y, L> const& yl, Sink<tag, S>&& sink)
        {
            return meld(yl.yield, meld(yl.link, FWD(sink)));
        }

        template<template<typename> typename tag, typename Y, typename L, typename S>
        auto meld(YieldLink<tag, Y, L> const& yl, Sink<tag, S>& sink)
        {
            return meld(yl.yield, meld(yl.link, sink));
        }

        // reorganize pipeline around sink
        template<template<typename> typename tag, typename L1, typename L2, typename S>
        LinkSink<tag, L1, LinkSink<tag, L2, S>> meld(LinkLink<tag, L1, L2>&& ll, Sink<tag, S>&& s)
        {
            return meld(FWD(ll).link1, meld(FWD(ll).link2, FWD(s)));
        }

        // reorganize pipeline around yield
        template<template<typename> typename tag, typename Y, typename L1, typename L2>
        YieldLink<tag, YieldLink<tag, Y, L1>, L2> meld(Yield<tag, Y>&& y, LinkLink<tag, L1, L2>&& ll)
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