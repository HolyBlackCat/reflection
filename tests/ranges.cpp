#include "em/refl/access/ranges.h"

#include <span>
#include <vector>

static_assert(em::Refl::Ranges::ShouldForwardElements<std::vector<int>>);
static_assert(!em::Refl::Ranges::ShouldForwardElements<std::span<int>>);
