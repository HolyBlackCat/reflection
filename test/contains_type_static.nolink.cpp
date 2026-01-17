#include "em/refl/for_each_matching_type_static.h"
#include "em/refl/macros/structs.h"
#include "em/refl/contains_type_static.h"

namespace
{
    template <typename T, typename Elem, em::Refl::IterationFlags Flags = {}>
    constexpr bool contains_type_static_using_foreach()
    {
        return (bool)em::Refl::ForEachStaticTypeMatchingPred<T, em::Refl::PredTypeMatchesElemCvref<Elem>, em::Meta::LoopAnyOf<>, Flags>(
            [&]<em::Meta::same_or_derived_from_and_cvref_convertible_to<Elem> U>()
            {
                return true;
            }
        );
    }


    template <typename T, typename Elem, em::Refl::IterationFlags Flags = {}>
    constexpr bool contains_type_static = []{
        constexpr bool a = em::Refl::TypeRecursivelyContainsStaticElemCvref<T, Elem, Flags>;
        constexpr bool b = contains_type_static_using_foreach<T, Elem, Flags>();
        static_assert(a <= b);
        static_assert(a >= b);
        return b;
    }();
}

struct X {};

struct A {};

struct B {EM_REFL()};

struct C
{
    EM_REFL(
        (X)(static x)
    )
};

struct D
{
    EM_REFL(
        (X)(a)
    )
};

struct E
{
    EM_REFL(
        (C)(static c)
    )
};

struct F
{
    EM_REFL(
        (C)(c)
    )
};

struct G
{
    EM_REFL(
        (F)(static f)
    )
};

struct H
{
    EM_REFL(
        (G)(g)
    )
};

static_assert(contains_type_static<X &, X &, em::Refl::IterationFlags::ignore_root> == false);
static_assert(contains_type_static<X &, X &>);

static_assert(contains_type_static<A &, X &, em::Refl::IterationFlags::ignore_root> == false);
static_assert(contains_type_static<A &, X &> == false);
static_assert(contains_type_static<B &, X &, em::Refl::IterationFlags::ignore_root> == false);
static_assert(contains_type_static<B &, X &> == false);
static_assert(contains_type_static<C &, X &, em::Refl::IterationFlags::ignore_root>);
static_assert(contains_type_static<C &, X &>);
static_assert(contains_type_static<D &, X &, em::Refl::IterationFlags::ignore_root> == false);
static_assert(contains_type_static<D &, X &> == false);
static_assert(contains_type_static<E &, X &, em::Refl::IterationFlags::ignore_root>);
static_assert(contains_type_static<E &, X &>);
static_assert(contains_type_static<F &, X &, em::Refl::IterationFlags::ignore_root>);
static_assert(contains_type_static<F &, X &>);
static_assert(contains_type_static<G &, X &, em::Refl::IterationFlags::ignore_root>);
static_assert(contains_type_static<G &, X &>);
static_assert(contains_type_static<H &, X &, em::Refl::IterationFlags::ignore_root>);
static_assert(contains_type_static<H &, X &>);


// Constness is ignored, other than on the root element.
static_assert(contains_type_static<const X &, X &, em::Refl::IterationFlags::ignore_root> == false);
static_assert(contains_type_static<const X &, X &> == false);
static_assert(contains_type_static<const C &, X &, em::Refl::IterationFlags::ignore_root>);
static_assert(contains_type_static<const C &, X &>);
