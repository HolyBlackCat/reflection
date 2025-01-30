#include "em/refl/access/ranges.h"

#include <span>
#include <vector>

static_assert(em::Refl::Ranges::TypeUnqualified<std::vector<int>>);
static_assert(!em::Refl::Ranges::TypeUnqualified<const std::vector<int>>);
static_assert(em::Refl::Ranges::Type<std::vector<int>>);
static_assert(em::Refl::Ranges::Type<const std::vector<int>>);
static_assert(em::Refl::Ranges::Type<const volatile std::vector<int> &&>);

static_assert(em::Refl::Ranges::ShouldForwardElements<std::vector<int>>);
static_assert(!em::Refl::Ranges::ShouldForwardElements<std::span<int>>);
