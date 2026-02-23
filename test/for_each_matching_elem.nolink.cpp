#include "em/refl/recursively_visit_elems.h"
#include "em/refl/macros/structs.h"

#include <forward_list>
#include <vector>

EM_STRUCT(A)
(
    (std::vector<int>)(a)
    (std::forward_list<float>)(b)
)

static_assert(!em::Meta::false_predicate::type<int>::value);

template <typename T, typename Elem, typename LoopBackend, em::Refl::IterationFlags Flags = {}>
concept CanIterate = requires(T &&t){em::Refl::RecursivelyVisitElemsOfTypeCvref<Elem, LoopBackend, Flags>(EM_FWD(t), [](Elem){});};

static_assert(CanIterate<A, int, em::Meta::LoopSimple>);
static_assert(CanIterate<A, float, em::Meta::LoopSimple>);
