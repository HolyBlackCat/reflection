#include "em/refl/access/bases.h"
#include "em/refl/macros/structs.h"

// A big part of those tests is copied from `detect_bases.cpp` from `em/meta`.

struct AA
{
    EM_REFL()
    struct A {EM_REFL()};
};

struct B : virtual AA::A
{
    EM_REFL()
};

struct C0 {EM_REFL()};

struct C : virtual AA::A, C0
{
    EM_REFL()
};

struct D : B, C
{
    EM_REFL()
};

static_assert(!em::Refl::Bases::HasBases<int>);
static_assert(em::Refl::Bases::HasBases<D>);
static_assert(em::Refl::Bases::HasBases<const volatile D &&>);

static_assert(em::Meta::lists_have_same_elems_and_size<em::Refl::Bases::AllBasesFlat                <D>, em::Meta::TypeList<AA::A, B, C0, C    >>);
static_assert(em::Meta::lists_have_same_elems_and_size<em::Refl::Bases::AllBasesFlatAndSelf         <D>, em::Meta::TypeList<AA::A, B, C0, C, D >>);
static_assert(em::Meta::lists_have_same_elems_and_size<em::Refl::Bases::VirtualBasesFlat            <D>, em::Meta::TypeList<AA::A              >>);
static_assert(em::Meta::lists_have_same_elems_and_size<em::Refl::Bases::VirtualBasesFlatAndSelf     <D>, em::Meta::TypeList<AA::A, D           >>);
static_assert(em::Meta::lists_have_same_elems_and_size<em::Refl::Bases::NonVirtualBasesFlat         <D>, em::Meta::TypeList<B, C0, C           >>);
static_assert(em::Meta::lists_have_same_elems_and_size<em::Refl::Bases::NonVirtualBasesFlatAndSelf  <D>, em::Meta::TypeList<B, C0, C, D        >>);
static_assert(em::Meta::lists_have_same_elems_and_size<em::Refl::Bases::NonVirtualBasesDirect       <D>, em::Meta::TypeList<B, C               >>);
static_assert(em::Meta::lists_have_same_elems_and_size<em::Refl::Bases::NonVirtualBasesDirectAndSelf<D>, em::Meta::TypeList<B, C, D            >>);

static_assert(em::Meta::lists_have_same_elems_and_size<em::Refl::Bases::AllBasesFlat<const volatile D &&>, em::Meta::TypeList<AA::A, B, C0, C>>);

// Edge cases:

// 1. Direct ambiguous base is skipped.
struct X1 : AA::A {EM_REFL()};
EM_SILENCE_INACCESSIBLE_BASE(
struct X2 : X1, AA::A {};
)
static_assert(em::Meta::lists_have_same_elems_and_size<em::Refl::Bases::NonVirtualBasesDirect<X2>, em::Meta::TypeList<X1>>);

// Both virtual and non-virtual base with the same name causes the virtual one to get skipped.
struct X3 : B, X1 {EM_REFL()};
static_assert(em::Meta::lists_have_same_elems_and_size<em::Refl::Bases::VirtualBasesFlat<X3>, em::Meta::TypeList<>>);


// ---

// Customizing the base list.

struct Custom {};

auto _adl_em_refl_Bases(int/*AdlDummy*/, const Custom *)
{
    struct S
    {
        static constexpr auto AllBasesFlat()          {return em::Meta::TypeList<int>{};}
        static constexpr auto VirtualBasesFlat()      {return em::Meta::TypeList<float>{};}
        static constexpr auto NonVirtualBasesFlat()   {return em::Meta::TypeList<double>{};}
        static constexpr auto NonVirtualBasesDirect() {return em::Meta::TypeList<char>{};}

        static constexpr bool HaveBases()
        {
            return true;
        }
    };
    return S{};
}
