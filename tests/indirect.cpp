#include "em/refl/access/indirect.h"

#include <memory>
#include <optional>
#include <vector>

static_assert(!em::Refl::Indirect::TypeUnqualified<int>);
static_assert(!em::Refl::Indirect::TypeUnqualified<const std::optional<int>>);
static_assert(em::Refl::Indirect::Type<const std::optional<int> &&>);

static_assert(em::Refl::Indirect::TypeUnqualified<std::optional<int>>);
static_assert(em::Refl::Indirect::TypeUnqualified<int *>);
static_assert(em::Refl::Indirect::TypeUnqualified<const int *>);
static_assert(em::Refl::Indirect::TypeUnqualified<std::unique_ptr<int>>);
static_assert(em::Refl::Indirect::TypeUnqualified<std::shared_ptr<int>>);
static_assert(em::Refl::Indirect::TypeUnqualified<std::vector<int>::iterator>);

static_assert(!em::Refl::Indirect::HasValue((int *)0));
static constexpr int x = 42;
static_assert(em::Refl::Indirect::HasValue(&x));
static_assert(&em::Refl::Indirect::GetValue(&x) == &x);
static_assert(em::Refl::Indirect::HasValue(std::vector<int>::iterator{}));
