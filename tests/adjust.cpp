#include "em/refl/access/adjust.h"
#include "em/meta/cvref.h"
#include "em/macros/utils/forward.h"

static_assert(em::Refl::Adjust::NeedsAdjustment<int> == false);
static_assert(em::Refl::Adjust::NeedsAdjustment<int &> == false);
static_assert(em::Refl::Adjust::NeedsAdjustment<int &&> == false);
static_assert(em::Refl::Adjust::NeedsAdjustment<const int> == false);
static_assert(em::Refl::Adjust::NeedsAdjustment<const int &> == false);
static_assert(em::Refl::Adjust::NeedsAdjustment<const int &&> == false);

struct A {};
struct B {A a; friend constexpr auto &&_adl_em_refl_ReflectAs(int, em::Meta::same_ignoring_cvref<B> auto &&self) {return EM_FWD(self).a;}};
struct C {B b; friend constexpr auto &&_adl_em_refl_ReflectAs(int, em::Meta::same_ignoring_cvref<C> auto &&self) {return EM_FWD(self).b;}};

// 0 adjustments.
static_assert(em::Refl::Adjust::NeedsAdjustment<A> == false);
static_assert(em::Refl::Adjust::NeedsAdjustment<A &> == false);
static_assert(em::Refl::Adjust::NeedsAdjustment<A &&> == false);
static_assert(em::Refl::Adjust::NeedsAdjustment<const A> == false);
static_assert(em::Refl::Adjust::NeedsAdjustment<const A &> == false);
static_assert(em::Refl::Adjust::NeedsAdjustment<const A &&> == false);

// 1 adjustment.
static_assert(em::Refl::Adjust::NeedsAdjustment<B> == true);
static_assert(em::Refl::Adjust::NeedsAdjustment<B &> == true);
static_assert(em::Refl::Adjust::NeedsAdjustment<B &&> == true);
static_assert(em::Refl::Adjust::NeedsAdjustment<const B> == true);
static_assert(em::Refl::Adjust::NeedsAdjustment<const B &> == true);
static_assert(em::Refl::Adjust::NeedsAdjustment<const B &&> == true);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustedType<B>, A &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustedType<B &>, A &>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustedType<B &&>, A &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustedType<const B>, const A &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustedType<const B &>, const A &>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustedType<const B &&>, const A &&>);

// Don't do repeated adjustments.
static_assert(std::is_same_v<em::Refl::Adjust::AdjustedType<C>, B &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustedType<C &>, B &>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustedType<C &&>, B &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustedType<const C>, const B &&>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustedType<const C &>, const B &>);
static_assert(std::is_same_v<em::Refl::Adjust::AdjustedType<const C &&>, const B &&>);
