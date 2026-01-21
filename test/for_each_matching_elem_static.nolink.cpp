#include "em/refl/for_each_matching_elem_static.h"
#include "em/refl/macros/structs.h"

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

void foo()
{
    em::Refl::ForEachStaticElemOfTypeCvref<H, X &>([](X &) {});
}
