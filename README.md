
# YLEMS

## What is it?

Repository for pipeline building

The name comes from two parts.
In the Big Bang theory ylem is the primordial matter of the universe.
And also I use terms Yield, Link, Sink, Meld and Element for some basic building blocks.
So abbreviation YLEMS is quite neat here and matches with goal.


## Abstract and Rules

Pipeline for ranges or pipes consists of following basic units:
* `Yield` - basically range or iterable - producer of values; 
* `Sink` - consumer of values;
* `Link` - some sort of transformator/filter of values; Link is in between `Yield` and `Sink`.

When we attach elements we `meld` them.

Herafter, for convenience we will denote melding with operator `/`.

Rules are:
1. Melded elements in form of `Yield/Link` is `Yield` type.
2. Melded elements in form of `Link/Link`  is `Link` type.
3. Melded elements in form of `Link/Sink`  is `Sink` type.
4. When connect `Yield/Sink` iteration starts.
5. `Sink` is preference. So, `(Yield/Link)/Sink` is rebuilt into `Yield/(Link/Sink)` before final iteration.

Having these rules one can write ones own piping-ranging DSLs.

## Notes

### Language restrictions

By now I rely on `C++17`.
This can be downgraded to `C++11`, I guess. But it is 2023 now.

### Mutability

In most cases I assume that `Yield` does const-iteration.

## Basic blocks

### tag base

We want to have pipeline with distibuishable DSLs.
That's why some sort of static polymorphysm must be used.
I chose to use one based on inheritance deduction.
This is less SFINAE but user elements implementation requires to publically inherit from special tag struct.

Since we may want to have separate pipelines with different precedence, we may have different tags for them.
That's why tag is obligatory in ylems namespace.

### Yield

Anything iterable can be `Yield`.

### Sink

Must provide at least one method `consume` either generic or specific with signature of form `bool consume(E e)`
with E be either value or reference.

Returned `bool` is signal for iteration to continue (`true`) or to stop (`false`).

### Link

If `Link` has some yield interaction, one may have ranges functionality for `Yield/Link`.
   ex. range based for.
To work right in case `Yield/Link` `Link` must provide generic methods begin(Y const&), end(Y const&) returning iterator and sentinel.
(iterator and sentinel are impelmentation details. FYI, Since C++17 sentinel type can have different type than iterator).

If Link has `Sink` interaction implemented one may have `Sink` functionality for `Link/Sink`.
To work right with `Link/Sink` `Link` must provide at least one specific or generic method in form `bool feed(S& sink, E e) const` 
with E be either value or reference. After this done `consume` is available. 

Returned `bool` is signal for iteration to continue (`true`) or to stop (`false`).


## Link Categories

The idea is to free user from writing boilerplate (if possible) so there are three basic categories for Links.
They are
* Filter      - skip values which do not satisfy condition;
* Transform   - obv. transform values and pass them on;
* TransformOr - either skip or pass transformed value; can be thought as meld of Transform and Filter but with some optimization.

## Elements

Some specific elements description can be seen [here](Elements.md)

## User code

### Defining user tag

For ylems to start working user must choose namespace where his tag will live.
Hereafter, tag is named `terminal`.

```C++
template<typename E> terminal: ylems::rules::_terminal_<E> {};
```



### Melding operation

For this there is macro
`YLEMS_MELD_OPERATION(tag, OP)`
genetating all overload set which is required.

Additionally for passing all sorts of rangeables into DSL
`YLEMS_MELD_RANGE_OPERATION(tag, OP)`
can be used.

For op you probably may want to be some binary overloadable operator.
Ex `operator/` or `operator|`. 
However, you may just have some named function if you want to. Ex `join`, `meld`.

!!! Macro used should be in same namespace where user tag is defined.

### Specyfying basic blocks and categories
**Note**. If you are not going to implement your own elements you may skip this.

For basic blocks one can use
```C++
    template<typename Y> using Yield = ylems::rules::Yield<Y, terminal>;
    template<typename L> using Link  = ylems::rules::Link<L, terminal>;
    template<typename S> using Sink  = ylems::rules::Sink<S, terminal>;
```

And for categories
```C++
    template<typename T> using Transform   = ylems::categories::Transform<T, terminal>;
    template<typename T> using TransformOr = ylems::categories::TransformOr<T, terminal>;
    template<typename T> using Filter      = ylems::categories::Filter<T, terminal>;
```

There is no restrictions on namespace here.

### Forwarding calls to ylems

Since ylems helper functions are tagged and you may want to write less on your user side,
you may want to write wrappers which forward calls to ylems. Like this.
In namespace where your terminal lives.

```C++
template<typename T, typename D, typename I>
                                 auto iota(T t, D d, I i) { return ylems::elements::iota<terminal>(t, d, i); }
template<typename T, typename D> auto iota(T t, D d) { return ylems::elements::iota<terminal>(t, d); }

template<typename T, typename I> auto linspace(T b, T e, I i) { return ylems::elements::linspace<terminal>(b, e, i); }

template<typename T>             auto range(T b, T e, T step) { return ylems::elements::range<terminal>(b, e, step); }
template< typename T>            auto range(T b, T e) { return ylems::elements::range<terminal>(b, e); }

template<typename Y>             auto yield(Y&& y) { return ylems::elements::yield<terminal>(FWD(y)); }
template<typename... T>          auto zip(T&&... f) { return ylems::elements::zip<terminal>(FWD(f)...); }

template<typename F>             auto filter(F&& f) { return ylems::elements::filter<terminal>(FWD(f)); }
template<typename I>             auto take(I i) { return ylems::elements::take<terminal>(i); }
template<typename I>             auto drop(I i) { return ylems::elements::drop<terminal>(i); }

template<typename F>             auto transform(F&& f) { return ylems::elements::transform<terminal>(FWD(f)); }
template<typename F>             auto transform_or(F&& f) { return ylems::elements::transform_or<terminal>(FWD(f)); }
template<typename F, typename G> auto transform_or(F&& f, G&& g) { return ylems::elements::transform_or<terminal>(FWD(f), FWD(g)); }
```

If you want to use those in another namespace you may put those functions there with `using` directive.

### User defined elements

If you want some specific class/struct to act like a link you may
add inheritance from one of categories or Link.

#### Categories

```C++
struct MyStruct: my_pipeline::categories::Transform<MyStruct>
{
    std::string operator()(double x) const { return std::to_string(2.0*x); }
}
```
All begin/end and consume methods will kick in by `categories::Transform`
which is assumed to be `Transform   = ylems::categories::Transform<T, terminal>;`

#### Categories with override

Imagine we want to have stringify all not nan values.
```C++
struct MyStruct: my_pipeline::categories::TransformOr<MyStruct>
{
    std::optional<std::string> operator()(double x) const 
    {
        if(x != x) return std::nullopt;
        return std::to_string(2.0*x);
    }

    template<typename S>
    bool feed(S& sink, double x) const
    {
        if(x != x)
            return true;
        return sink.consume(x);
    }

    // for begin and end TransformOr<MyStruct> will be used
}
```

#### Totally user defined 

In this case declaration will probably look like
```C++
struct MyStruct: my_pipeline::rules::Link<MyStruct>
```
and user must provide `begin`/`end` and `feed` methods.

If there is actually category fitting for user defined element it is still recomended to use category.
