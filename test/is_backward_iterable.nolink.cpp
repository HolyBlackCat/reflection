#include "em/refl/macros/structs.h"
#include "em/refl/recursively_visit_elems.h"

#include <forward_list>
#include <vector>


EM_STRUCT(A)
(
    (std::vector<int>)(v)
)

EM_STRUCT(B)
(
    (std::forward_list<int>)(v)
)

static_assert(em::Refl::RecursivelyBackwardIterable<std::vector<int>>);
static_assert(!em::Refl::RecursivelyBackwardIterable<std::forward_list<int>>);
static_assert(em::Refl::RecursivelyBackwardIterable<int>);

static_assert(em::Refl::RecursivelyBackwardIterable<A>);
static_assert(!em::Refl::RecursivelyBackwardIterable<B>);
