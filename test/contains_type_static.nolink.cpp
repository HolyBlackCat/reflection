#include "em/refl/macros/structs.h"
#include "em/refl/contains_type_static.h"

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

static_assert(em::Refl::TypeRecursivelyContainsStaticElemCvref<X &, X &, em::Refl::IterationFlags::ignore_root> == false);
static_assert(em::Refl::TypeRecursivelyContainsStaticElemCvref<X &, X &>);

static_assert(em::Refl::TypeRecursivelyContainsStaticElemCvref<A &, X &, em::Refl::IterationFlags::ignore_root> == false);
static_assert(em::Refl::TypeRecursivelyContainsStaticElemCvref<A &, X &> == false);
static_assert(em::Refl::TypeRecursivelyContainsStaticElemCvref<B &, X &, em::Refl::IterationFlags::ignore_root> == false);
static_assert(em::Refl::TypeRecursivelyContainsStaticElemCvref<B &, X &> == false);
static_assert(em::Refl::TypeRecursivelyContainsStaticElemCvref<C &, X &, em::Refl::IterationFlags::ignore_root>);
static_assert(em::Refl::TypeRecursivelyContainsStaticElemCvref<C &, X &>);
static_assert(em::Refl::TypeRecursivelyContainsStaticElemCvref<D &, X &, em::Refl::IterationFlags::ignore_root> == false);
static_assert(em::Refl::TypeRecursivelyContainsStaticElemCvref<D &, X &> == false);
static_assert(em::Refl::TypeRecursivelyContainsStaticElemCvref<E &, X &, em::Refl::IterationFlags::ignore_root>);
static_assert(em::Refl::TypeRecursivelyContainsStaticElemCvref<E &, X &>);
static_assert(em::Refl::TypeRecursivelyContainsStaticElemCvref<F &, X &, em::Refl::IterationFlags::ignore_root>);
static_assert(em::Refl::TypeRecursivelyContainsStaticElemCvref<F &, X &>);
static_assert(em::Refl::TypeRecursivelyContainsStaticElemCvref<G &, X &, em::Refl::IterationFlags::ignore_root>);
static_assert(em::Refl::TypeRecursivelyContainsStaticElemCvref<G &, X &>);
static_assert(em::Refl::TypeRecursivelyContainsStaticElemCvref<H &, X &, em::Refl::IterationFlags::ignore_root>);
static_assert(em::Refl::TypeRecursivelyContainsStaticElemCvref<H &, X &>);


// Constness is ignored, other than on the root element.
static_assert(em::Refl::TypeRecursivelyContainsStaticElemCvref<const X &, X &, em::Refl::IterationFlags::ignore_root> == false);
static_assert(em::Refl::TypeRecursivelyContainsStaticElemCvref<const X &, X &> == false);
static_assert(em::Refl::TypeRecursivelyContainsStaticElemCvref<const C &, X &, em::Refl::IterationFlags::ignore_root>);
static_assert(em::Refl::TypeRecursivelyContainsStaticElemCvref<const C &, X &>);
