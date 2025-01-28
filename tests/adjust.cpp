#include "em/refl/access/adjust.h"
#include "em/meta/cvref.h"

static_assert(em::Refl::Adjust::NeedsAdjustment<int> == false);
static_assert(em::Refl::Adjust::NeedsAdjustment<int &> == false);
static_assert(em::Refl::Adjust::NeedsAdjustment<int &&> == false);
static_assert(em::Refl::Adjust::NeedsAdjustment<const int> == false);
static_assert(em::Refl::Adjust::NeedsAdjustment<const int &> == false);
static_assert(em::Refl::Adjust::NeedsAdjustment<const int &&> == false);

// Here we're NOT adjusting non-references const rvalue references by themselves. That only happens if the callback returns one.
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<int>, int &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<int &>, int &>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<int &&>, int &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<const int>, const int &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<const int &>, const int &>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<const int &&>, const int &&>);

// Test that adjusting to a non-ref replaces that with a const rvalue reference.
struct AdjustsToNonref {friend auto _adl_em_refl_ReflectAs(int, AdjustsToNonref &) {return 42;}};
struct AdjustsToRef {int x = 42; friend auto &_adl_em_refl_ReflectAs(int, AdjustsToRef &self) {return self.x;}};
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<AdjustsToNonref &>, const int &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<AdjustsToRef &>, int &>);

struct A {};
struct B {A a; friend constexpr auto &&_adl_em_refl_ReflectAs(int, em::Meta::same_ignoring_cvref<B> auto &&self) {return EM_FWD(self).a;}};
struct C {B b; friend constexpr auto &&_adl_em_refl_ReflectAs(int, em::Meta::same_ignoring_cvref<C> auto &&self) {return EM_FWD(self).b;}};
struct D {C c; friend constexpr auto &&_adl_em_refl_ReflectAs(int, em::Meta::same_ignoring_cvref<D> auto &&self) {return EM_FWD(self).c;}};

// Identity adjustment does nothing.
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<A>, A &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<A &>, A &>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<A &&>, A &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<const A>, const A &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<const A &>, const A &>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<const A &&>, const A &&>);
static_assert(em::Refl::Adjust::NeedsAdjustment<A> == false);
static_assert(em::Refl::Adjust::NeedsAdjustment<A &> == false);
static_assert(em::Refl::Adjust::NeedsAdjustment<A &&> == false);
static_assert(em::Refl::Adjust::NeedsAdjustment<const A> == false);
static_assert(em::Refl::Adjust::NeedsAdjustment<const A &> == false);
static_assert(em::Refl::Adjust::NeedsAdjustment<const A &&> == false);

// 1 adjustment.
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<B>, A &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<B &>, A &>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<B &&>, A &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<const B>, const A &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<const B &>, const A &>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<const B &&>, const A &&>);
static_assert(em::Refl::Adjust::NeedsAdjustment<B> == true);
static_assert(em::Refl::Adjust::NeedsAdjustment<B &> == true);
static_assert(em::Refl::Adjust::NeedsAdjustment<B &&> == true);
static_assert(em::Refl::Adjust::NeedsAdjustment<const B> == true);
static_assert(em::Refl::Adjust::NeedsAdjustment<const B &> == true);
static_assert(em::Refl::Adjust::NeedsAdjustment<const B &&> == true);

// 2 adjustments.
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<C>, A &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<C &>, A &>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<C &&>, A &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<const C>, const A &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<const C &>, const A &>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<const C &&>, const A &&>);

// 3 adjustments.
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<C>, A &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<C &>, A &>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<C &&>, A &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<const C>, const A &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<const C &>, const A &>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<const C &&>, const A &&>);


// Adjustments with predicates:

template <typename T> struct AorC : std::bool_constant<em::Meta::same_ignoring_cvref<T, A> || em::Meta::same_ignoring_cvref<T, C>> {};
template <typename T> struct BorD : std::bool_constant<em::Meta::same_ignoring_cvref<T, B> || em::Meta::same_ignoring_cvref<T, D>> {};
template <typename T> struct CorD : std::bool_constant<em::Meta::same_ignoring_cvref<T, C> || em::Meta::same_ignoring_cvref<T, D>> {};

static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<D, AorC>, A &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<D &, AorC>, A &>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<D &&, AorC>, A &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<const D, AorC>, const A &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<const D &, AorC>, const A &>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<const D &&, AorC>, const A &&>);

static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<D, BorD>, B &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<D &, BorD>, B &>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<D &&, BorD>, B &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<const D, BorD>, const B &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<const D &, BorD>, const B &>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<const D &&, BorD>, const B &&>);

static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<D, CorD>, C &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<D &, CorD>, C &>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<D &&, CorD>, C &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<const D, CorD>, const C &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<const D &, CorD>, const C &>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<const D &&, CorD>, const C &&>);

static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<D, em::Meta::false_trait>, D &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<D &, em::Meta::false_trait>, D &>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<D &&, em::Meta::false_trait>, D &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<const D, em::Meta::false_trait>, const D &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<const D &, em::Meta::false_trait>, const D &>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustType<const D &&, em::Meta::false_trait>, const D &&>);

// Now `NeedsAdjustment` with predicates.
static_assert(em::Refl::Adjust::NeedsAdjustment<B> == true);
static_assert(em::Refl::Adjust::NeedsAdjustment<B, BorD> == false);
static_assert(em::Refl::Adjust::NeedsAdjustment<B, AorC> == true);

// Calling `Adjust()`:

D d;
static_assert(em::Refl::Adjust::Adjust(D{},                          [](std::same_as<A        > auto &&){return true;}));
static_assert(em::Refl::Adjust::Adjust(static_cast<const D &&>(D{}), [](std::same_as<const A  > auto &&){return true;}));
static_assert(em::Refl::Adjust::Adjust(d,                            [](std::same_as<A &       > auto &&){return true;}));
static_assert(em::Refl::Adjust::Adjust(std::as_const(d),             [](std::same_as<const A & > auto &&){return true;}));
static_assert(em::Refl::Adjust::Adjust<AorC>(D{},                          [](std::same_as<A        > auto &&){return true;}));
static_assert(em::Refl::Adjust::Adjust<AorC>(static_cast<const D &&>(D{}), [](std::same_as<const A  > auto &&){return true;}));
static_assert(em::Refl::Adjust::Adjust<AorC>(d,                            [](std::same_as<A &      > auto &&){return true;}));
static_assert(em::Refl::Adjust::Adjust<AorC>(std::as_const(d),             [](std::same_as<const A &> auto &&){return true;}));
static_assert(em::Refl::Adjust::Adjust<BorD>(D{},                          [](std::same_as<B        > auto &&){return true;}));
static_assert(em::Refl::Adjust::Adjust<BorD>(static_cast<const D &&>(D{}), [](std::same_as<const B  > auto &&){return true;}));
static_assert(em::Refl::Adjust::Adjust<BorD>(d,                            [](std::same_as<B &      > auto &&){return true;}));
static_assert(em::Refl::Adjust::Adjust<BorD>(std::as_const(d),             [](std::same_as<const B &> auto &&){return true;}));
static_assert(em::Refl::Adjust::Adjust<CorD>(D{},                          [](std::same_as<C        > auto &&){return true;}));
static_assert(em::Refl::Adjust::Adjust<CorD>(static_cast<const D &&>(D{}), [](std::same_as<const C  > auto &&){return true;}));
static_assert(em::Refl::Adjust::Adjust<CorD>(d,                            [](std::same_as<C &      > auto &&){return true;}));
static_assert(em::Refl::Adjust::Adjust<CorD>(std::as_const(d),             [](std::same_as<const C &> auto &&){return true;}));
static_assert(em::Refl::Adjust::Adjust<em::Meta::false_trait>(D{},                          [](std::same_as<D        > auto &&){return true;}));
static_assert(em::Refl::Adjust::Adjust<em::Meta::false_trait>(static_cast<const D &&>(D{}), [](std::same_as<const D  > auto &&){return true;}));
static_assert(em::Refl::Adjust::Adjust<em::Meta::false_trait>(d,                            [](std::same_as<D &      > auto &&){return true;}));
static_assert(em::Refl::Adjust::Adjust<em::Meta::false_trait>(std::as_const(d),             [](std::same_as<const D &> auto &&){return true;}));
