#include "em/refl/macros/structs.h"
#include "em/refl/access/structs.h"

class A
{
    EM_REFL(
        (([[no_unique_address]]) int)(x) // While we're at it, test that custom additions to the declarations work.
      EM_PRIVATE
      EM_REFL_ANNOTATE_LOW(blah,, 42, 43) // The second argument is empty, so we don't error despite this being an unused annotation.
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

static_assert(em::Refl::Structs::num_static_members<A> == 0);
static_assert(em::Refl::Structs::num_static_members<const volatile A &&> == 0);

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

static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<A, 1>, float &&>);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<A &, 1>, float &>);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<A &&, 1>, float &&>);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<const A, 1>, const float &&>);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<const A &, 1>, const float &>);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<const A &&, 1>, const float &&>);

static_assert(em::Refl::Structs::HasMemberNames<A>);
static_assert(em::Refl::Structs::HasMemberNames<const volatile A &&>);
static_assert(em::Refl::Structs::GetMemberName<A>(0) == "x");
static_assert(em::Refl::Structs::GetMemberName<const volatile A &&>(1) == "y");

struct A1 : em::Refl::BasicAttribute {};
struct A2 : em::Refl::BasicAttribute {};
struct A2_ : A2 {};
struct A3 : em::Refl::BasicAttribute {};

struct Attrs
{
    EM_REFL(
        (int &)(ref,)
        (int, A2, A1)(attrs)
        (int, A2, A2_)(attrs2)
        (int, A2_)(attrs3)
    )
};

static_assert(std::is_same_v<em::Refl::Structs::MemberType<Attrs, 0>, int &>);
static_assert(std::is_same_v<em::Refl::Structs::MemberAttributes<Attrs, 0>, em::Meta::TypeList<>>);
static_assert(std::is_same_v<em::Refl::Structs::MemberAttributes<Attrs, 1>, em::Meta::TypeList<A2, A1>>);
static_assert(std::is_same_v<em::Refl::Structs::MemberAttributes<const volatile Attrs &&, 0>, em::Meta::TypeList<>>);
static_assert(std::is_same_v<em::Refl::Structs::MemberAttributes<const volatile Attrs &&, 1>, em::Meta::TypeList<A2, A1>>);
static_assert(!em::Refl::Structs::member_has_attribute<Attrs, 0, A1>);
static_assert(!em::Refl::Structs::member_has_attribute<const volatile Attrs &&, 0, A1>);
static_assert(em::Refl::Structs::member_has_attribute<Attrs, 1, A1>);
static_assert(em::Refl::Structs::member_has_attribute<Attrs, 1, A2>);
static_assert(em::Refl::Structs::member_has_attribute<Attrs, 2, A2_>);
// static_assert(em::Refl::Structs::member_has_attribute<Attrs, 2, A2>); // Hard error because of the ambiguity.
static_assert(std::is_same_v<em::Refl::Structs::MemberFindAttribute<Attrs, 3, A2_>, A2_>);
static_assert(std::is_same_v<em::Refl::Structs::MemberFindAttribute<Attrs, 3, A2>, A2_>);


// Unnamed members!

struct UnnamedMembers
{
    EM_REFL(
        EM_UNNAMED_MEMBERS
    )
};
static_assert(!em::Refl::Structs::HasMemberNames<UnnamedMembers>);

struct UnnamedMembers2
{
    EM_REFL(
        EM_UNNAMED_MEMBERS
        (int)(x)
        (float)(y)
    )
};
static_assert(!em::Refl::Structs::HasMemberNames<UnnamedMembers2>);


// Empty classes!
class Empty
{
    EM_REFL()
};
static_assert(em::Refl::Structs::TypeUnqualified<Empty>);
static_assert(em::Refl::Structs::num_members<Empty> == 0);
static_assert(em::Refl::Structs::num_static_members<Empty> == 0);


// Standard tuples!
static_assert(em::Refl::Structs::TypeUnqualified<std::tuple<int, float &>>);
static_assert(!em::Refl::Structs::TypeUnqualified<const std::tuple<int, float &>>);
static_assert(em::Refl::Structs::Type<std::tuple<int, float &>>);
static_assert(em::Refl::Structs::Type<const std::tuple<int, float &>>);
static_assert(em::Refl::Structs::Type<const volatile std::tuple<int, float &> &&>);
static_assert(!em::Refl::Structs::HasMemberNames<std::tuple<int, float &>>);

static_assert(em::Refl::Structs::num_members<std::tuple<int, float &>> == 2);
static_assert(em::Refl::Structs::num_static_members<std::tuple<int, float &>> == 0);

static_assert(std::is_same_v<em::Refl::Structs::MemberType<std::tuple<int, float &>, 0>, int>);
static_assert(std::is_same_v<em::Refl::Structs::MemberType<std::tuple<int, float &>, 1>, float &>);


// Static members!

class B
{
    EM_REFL(
        (int)(static x)
      EM_PRIVATE
        (float)(inline static y)
    )
};

static_assert(em::Refl::Structs::TypeUnqualified<B>);
static_assert(!em::Refl::Structs::TypeUnqualified<const B>);
static_assert(em::Refl::Structs::Type<const B>);
static_assert(em::Refl::Structs::Type<const volatile B &&>);
static_assert(!em::Refl::Structs::Type<int>);

static_assert(em::Refl::Structs::num_members<B> == 0);
static_assert(em::Refl::Structs::num_static_members<B> == 2);
static_assert(em::Refl::Structs::num_static_members<const volatile B &&> == 2);

static_assert(std::is_same_v<decltype(em::Refl::Structs::GetStaticMemberConst<B, 1>()), const float & >);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetStaticMemberMutable<B, 1>()), float & >);

static_assert(std::is_same_v<em::Refl::Structs::StaticMemberType<B, 0>, int>);
static_assert(std::is_same_v<em::Refl::Structs::StaticMemberType<const volatile B &&, 1>, float>);

static_assert(std::is_same_v<em::Refl::Structs::StaticMemberTypeCvrefConst<B, 1>, const float &>);
static_assert(std::is_same_v<em::Refl::Structs::StaticMemberTypeCvrefConst<const B &&, 1>, const float &>);
static_assert(std::is_same_v<em::Refl::Structs::StaticMemberTypeCvrefMutable<B, 1>, float &>);
static_assert(std::is_same_v<em::Refl::Structs::StaticMemberTypeCvrefMutable<const B &&, 1>, float &>);

static_assert(em::Refl::Structs::GetStaticMemberName<B>(0) == "x");
static_assert(em::Refl::Structs::GetStaticMemberName<const volatile B &&>(1) == "y");

struct StaticAttrs
{
    EM_REFL(
        (int &)(static ref,)
        (int, A2, A1)(static attrs)
        (int, A2, A2_)(static attrs2)
        (int, A2_)(static attrs3)
    )
};

static_assert(std::is_same_v<em::Refl::Structs::StaticMemberType<StaticAttrs, 0>, int &>);
static_assert(std::is_same_v<em::Refl::Structs::StaticMemberAttributes<StaticAttrs, 0>, em::Meta::TypeList<>>);
static_assert(std::is_same_v<em::Refl::Structs::StaticMemberAttributes<StaticAttrs, 1>, em::Meta::TypeList<A2, A1>>);
static_assert(std::is_same_v<em::Refl::Structs::StaticMemberAttributes<const volatile StaticAttrs &&, 0>, em::Meta::TypeList<>>);
static_assert(std::is_same_v<em::Refl::Structs::StaticMemberAttributes<const volatile StaticAttrs &&, 1>, em::Meta::TypeList<A2, A1>>);
static_assert(!em::Refl::Structs::static_member_has_attribute<StaticAttrs, 0, A1>);
static_assert(!em::Refl::Structs::static_member_has_attribute<const volatile StaticAttrs &&, 0, A1>);
static_assert(em::Refl::Structs::static_member_has_attribute<StaticAttrs, 1, A1>);
static_assert(em::Refl::Structs::static_member_has_attribute<StaticAttrs, 1, A2>);
static_assert(em::Refl::Structs::static_member_has_attribute<StaticAttrs, 2, A2_>);
// static_assert(em::Refl::Structs::static_member_has_attribute<StaticAttrs, 2, A2>); // Hard error because of the ambiguity.
static_assert(std::is_same_v<em::Refl::Structs::StaticMemberFindAttribute<StaticAttrs, 3, A2_>, A2_>);
static_assert(std::is_same_v<em::Refl::Structs::StaticMemberFindAttribute<StaticAttrs, 3, A2>, A2_>);


// Propagating cvref.

EM_STRUCT( Cvref )
(
    (int)(i)
    (int &)(ir,)
    (int &&)(irr)
    (const int)(ci)
    (const int &)(cir)
    (const int &&)(cirr)
    (int)(static si)
    (int &)(static sir)
    (int &&)(static sirr)
    (const int)(static sci)
    (const int &)(static scir)
    (const int &&)(static scirr)
)


static_assert(std::is_same_v<em::Refl::Structs::MemberType<Cvref, 0>, int>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst  <0>(std::declval<      Cvref & >())), const int & >);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst  <0>(std::declval<      Cvref &&>())), const int &&>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst  <0>(std::declval<const Cvref & >())), const int & >);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst  <0>(std::declval<const Cvref &&>())), const int &&>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<0>(std::declval<      Cvref & >())),       int & >);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<0>(std::declval<      Cvref &&>())),       int &&>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<0>(std::declval<const Cvref & >())), const int & >);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<0>(std::declval<const Cvref &&>())), const int &&>);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<      Cvref & , 0>,       int & >);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<      Cvref &&, 0>,       int &&>);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<const Cvref & , 0>, const int & >);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<const Cvref &&, 0>, const int &&>);

static_assert(std::is_same_v<em::Refl::Structs::MemberType<Cvref, 1>, int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst  <1>(std::declval<      Cvref & >())), int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst  <1>(std::declval<      Cvref &&>())), int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst  <1>(std::declval<const Cvref & >())), int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst  <1>(std::declval<const Cvref &&>())), int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<1>(std::declval<      Cvref & >())), int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<1>(std::declval<      Cvref &&>())), int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<1>(std::declval<const Cvref & >())), int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<1>(std::declval<const Cvref &&>())), int &>);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<      Cvref & , 1>, int & >);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<      Cvref &&, 1>, int & >);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<const Cvref & , 1>, int & >);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<const Cvref &&, 1>, int & >);

static_assert(std::is_same_v<em::Refl::Structs::MemberType<Cvref, 2>, int &&>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst  <2>(std::declval<      Cvref & >())), int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst  <2>(std::declval<      Cvref &&>())), int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst  <2>(std::declval<const Cvref & >())), int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst  <2>(std::declval<const Cvref &&>())), int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<2>(std::declval<      Cvref & >())), int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<2>(std::declval<      Cvref &&>())), int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<2>(std::declval<const Cvref & >())), int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<2>(std::declval<const Cvref &&>())), int &>);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<      Cvref & , 2>, int & >);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<      Cvref &&, 2>, int & >);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<const Cvref & , 2>, int & >);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<const Cvref &&, 2>, int & >);


static_assert(std::is_same_v<em::Refl::Structs::MemberType<Cvref, 3>, const int>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst  <3>(std::declval<      Cvref & >())), const int & >);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst  <3>(std::declval<      Cvref &&>())), const int &&>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst  <3>(std::declval<const Cvref & >())), const int & >);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst  <3>(std::declval<const Cvref &&>())), const int &&>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<3>(std::declval<      Cvref & >())), const int & >);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<3>(std::declval<      Cvref &&>())), const int &&>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<3>(std::declval<const Cvref & >())), const int & >);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<3>(std::declval<const Cvref &&>())), const int &&>);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<      Cvref & , 3>, const int & >);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<      Cvref &&, 3>, const int &&>);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<const Cvref & , 3>, const int & >);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<const Cvref &&, 3>, const int &&>);

static_assert(std::is_same_v<em::Refl::Structs::MemberType<Cvref, 4>, const int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst  <4>(std::declval<      Cvref & >())), const int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst  <4>(std::declval<      Cvref &&>())), const int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst  <4>(std::declval<const Cvref & >())), const int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst  <4>(std::declval<const Cvref &&>())), const int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<4>(std::declval<      Cvref & >())), const int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<4>(std::declval<      Cvref &&>())), const int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<4>(std::declval<const Cvref & >())), const int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<4>(std::declval<const Cvref &&>())), const int &>);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<      Cvref & , 4>, const int & >);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<      Cvref &&, 4>, const int & >);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<const Cvref & , 4>, const int & >);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<const Cvref &&, 4>, const int & >);

static_assert(std::is_same_v<em::Refl::Structs::MemberType<Cvref, 5>, const int &&>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst  <5>(std::declval<      Cvref & >())), const int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst  <5>(std::declval<      Cvref &&>())), const int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst  <5>(std::declval<const Cvref & >())), const int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberConst  <5>(std::declval<const Cvref &&>())), const int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<5>(std::declval<      Cvref & >())), const int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<5>(std::declval<      Cvref &&>())), const int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<5>(std::declval<const Cvref & >())), const int &>);
static_assert(std::is_same_v<decltype(em::Refl::Structs::GetMemberMutable<5>(std::declval<const Cvref &&>())), const int &>);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<      Cvref & , 5>, const int & >);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<      Cvref &&, 5>, const int & >);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<const Cvref & , 5>, const int & >);
static_assert(std::is_same_v<em::Refl::Structs::MemberTypeCvref<const Cvref &&, 5>, const int & >);
