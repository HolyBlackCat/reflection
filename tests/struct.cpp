#include "em/refl/macros/struct.h"
#include "em/refl/access/struct.h"

class A
{
    EM_REFL(
        (int)(x)
        (float)(y,42)
    )
};

static_assert(em::Refl::Struct::Type<A>);
static_assert(!em::Refl::Struct::Type<const A>);
static_assert(!em::Refl::Struct::Type<int>);
static_assert(em::Refl::Struct::TypeMaybeCvref<const A>);
static_assert(em::Refl::Struct::TypeMaybeCvref<const volatile A &&>);
static_assert(!em::Refl::Struct::TypeMaybeCvref<int>);

static_assert(em::Refl::Struct::num_members<A> == 2);
static_assert(em::Refl::Struct::num_members<const volatile A &&> == 2);

static_assert(std::is_same_v<decltype(em::Refl::Struct::GetMemberConst<1>(std::declval<      A & >())), const float & >);
static_assert(std::is_same_v<decltype(em::Refl::Struct::GetMemberConst<1>(std::declval<      A &&>())), const float &&>);
static_assert(std::is_same_v<decltype(em::Refl::Struct::GetMemberConst<1>(std::declval<const A & >())), const float & >);
static_assert(std::is_same_v<decltype(em::Refl::Struct::GetMemberConst<1>(std::declval<const A &&>())), const float &&>);

static_assert(std::is_same_v<decltype(em::Refl::Struct::GetMemberMutable<1>(std::declval<      A & >())),       float & >);
static_assert(std::is_same_v<decltype(em::Refl::Struct::GetMemberMutable<1>(std::declval<      A &&>())),       float &&>);
static_assert(std::is_same_v<decltype(em::Refl::Struct::GetMemberMutable<1>(std::declval<const A & >())), const float & >);
static_assert(std::is_same_v<decltype(em::Refl::Struct::GetMemberMutable<1>(std::declval<const A &&>())), const float &&>);

static_assert(std::is_same_v<em::Refl::Struct::MemberType<A, 0>, int>);
static_assert(std::is_same_v<em::Refl::Struct::MemberType<const volatile A &&, 1>, float>);

struct A1 : em::Refl::BasicAttribute {};
struct A2 : em::Refl::BasicAttribute {};
struct A2_ : A2 {};
struct A3 : em::Refl::BasicAttribute {};

struct Extras
{
    EM_REFL(
        (int &)(ref,)
        (int, A2, A1)(attrs)
        (int, A2, A2_)(attrs2)
        (int, A2_)(attrs3)
    )
};

static_assert(std::is_same_v<em::Refl::Struct::MemberType<Extras, 0>, int &>);
static_assert(std::is_same_v<em::Refl::Struct::MemberAttributes<Extras, 0>, em::Meta::TypeList<>>);
static_assert(std::is_same_v<em::Refl::Struct::MemberAttributes<Extras, 1>, em::Meta::TypeList<A2, A1>>);
static_assert(std::is_same_v<em::Refl::Struct::MemberAttributes<const volatile Extras &&, 0>, em::Meta::TypeList<>>);
static_assert(std::is_same_v<em::Refl::Struct::MemberAttributes<const volatile Extras &&, 1>, em::Meta::TypeList<A2, A1>>);
static_assert(!em::Refl::Struct::member_has_attribute<Extras, 0, A1>);
static_assert(!em::Refl::Struct::member_has_attribute<const volatile Extras &&, 0, A1>);
static_assert(em::Refl::Struct::member_has_attribute<Extras, 1, A1>);
static_assert(em::Refl::Struct::member_has_attribute<Extras, 1, A2>);
static_assert(em::Refl::Struct::member_has_attribute<Extras, 2, A2_>);
// static_assert(em::Refl::Struct::member_has_attribute<Extras, 2, A2>); // Hard error because of the ambiguity.
static_assert(std::is_same_v<em::Refl::Struct::MemberFindAttribute<Extras, 3, A2_>, A2_>);
static_assert(std::is_same_v<em::Refl::Struct::MemberFindAttribute<Extras, 3, A2>, A2_>);
