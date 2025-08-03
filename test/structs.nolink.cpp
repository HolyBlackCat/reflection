#include "em/refl/macros/structs.h"
#include "em/refl/access/structs.h"

class A
{
    EM_REFL(
        (int)(x)
      EM_PRIVATE
        // Test that leading non-letters work in the type.
        (::std::type_identity_t<float>)(y,42)
    )
};

static_assert(em::Refl::Structs::TypeUnqualified<A>);
static_assert(!em::Refl::Structs::TypeUnqualified<const A>);
static_assert(!em::Refl::Structs::TypeUnqualified<int>);
static_assert(em::Refl::Structs::Type<const A>);
static_assert(em::Refl::Structs::Type<const volatile A &&>);
static_assert(!em::Refl::Structs::Type<int>);

static_assert(em::Refl::Structs::num_members<A> == 2);
static_assert(em::Refl::Structs::num_members<const volatile A &&> == 2);

static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst<1>(std::declval<      A & >())), const float & >);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst<1>(std::declval<      A &&>())), const float &&>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst<1>(std::declval<const A & >())), const float & >);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst<1>(std::declval<const A &&>())), const float &&>);

static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<1>(std::declval<      A & >())),       float & >);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<1>(std::declval<      A &&>())),       float &&>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<1>(std::declval<const A & >())), const float & >);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<1>(std::declval<const A &&>())), const float &&>);

static_assert(std::is_same_v<em::Refl::Structs::MemberType<A, 0>, int>);
static_assert(std::is_same_v<em::Refl::Structs::MemberType<const volatile A &&, 1>, float>);

static_assert(em::Refl::Structs::HasMemberNames<A>);
static_assert(em::Refl::Structs::HasMemberNames<const volatile A &&>);
static_assert(em::Refl::Structs::GetMemberName<A>(0) == "x");
static_assert(em::Refl::Structs::GetMemberName<const volatile A &&>(1) == "y");

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

static_assert(std::is_same_v<em::Refl::Structs::MemberType<Extras, 0>, int &>);
static_assert(std::is_same_v<em::Refl::Structs::MemberAttributes<Extras, 0>, em::Meta::TypeList<>>);
static_assert(std::is_same_v<em::Refl::Structs::MemberAttributes<Extras, 1>, em::Meta::TypeList<A2, A1>>);
static_assert(std::is_same_v<em::Refl::Structs::MemberAttributes<const volatile Extras &&, 0>, em::Meta::TypeList<>>);
static_assert(std::is_same_v<em::Refl::Structs::MemberAttributes<const volatile Extras &&, 1>, em::Meta::TypeList<A2, A1>>);
static_assert(!em::Refl::Structs::member_has_attribute<Extras, 0, A1>);
static_assert(!em::Refl::Structs::member_has_attribute<const volatile Extras &&, 0, A1>);
static_assert(em::Refl::Structs::member_has_attribute<Extras, 1, A1>);
static_assert(em::Refl::Structs::member_has_attribute<Extras, 1, A2>);
static_assert(em::Refl::Structs::member_has_attribute<Extras, 2, A2_>);
// static_assert(em::Refl::Structs::member_has_attribute<Extras, 2, A2>); // Hard error because of the ambiguity.
static_assert(std::is_same_v<em::Refl::Structs::MemberFindAttribute<Extras, 3, A2_>, A2_>);
static_assert(std::is_same_v<em::Refl::Structs::MemberFindAttribute<Extras, 3, A2>, A2_>);


// Empty classes!
class Empty
{
    EM_REFL()
};
static_assert(em::Refl::Structs::TypeUnqualified<Empty>);
static_assert(em::Refl::Structs::num_members<Empty> == 0);


// Standard tuples!
static_assert(em::Refl::Structs::TypeUnqualified<std::tuple<int, float &>>);
static_assert(!em::Refl::Structs::TypeUnqualified<const std::tuple<int, float &>>);
static_assert(em::Refl::Structs::Type<std::tuple<int, float &>>);
static_assert(em::Refl::Structs::Type<const std::tuple<int, float &>>);
static_assert(em::Refl::Structs::Type<const volatile std::tuple<int, float &> &&>);
static_assert(!em::Refl::Structs::HasMemberNames<std::tuple<int, float &>>);

static_assert(em::Refl::Structs::num_members<std::tuple<int, float &>> == 2);

static_assert(std::is_same_v<em::Refl::Structs::MemberType<std::tuple<int, float &>, 0>, int>);
static_assert(std::is_same_v<em::Refl::Structs::MemberType<std::tuple<int, float &>, 1>, float &>);
