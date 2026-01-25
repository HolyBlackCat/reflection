#pragma once

#include "em/meta/const_for.h"
#include "em/refl/recursively_visit_types_static.h"
#include "em/refl/recursively_visit_types.h"

namespace em::Refl
{
    // Combines visiting non-static and static types.
    template <typename T, Meta::TypePredicate Pred, Meta::LoopBackendType LoopBackend = Meta::LoopSimple, IterationFlags Flags = {}, Meta::Deduce..., typename F>
    constexpr decltype(auto) ForEachMaybeStaticTypeMatchingPred(F &&func)
    {
        return Meta::RunEachFunc<LoopBackend>(
            [&]<typename Pred2 = Pred>()
            {
                return (RecursivelyVisitTypesMatchingPred<T, Pred2, LoopBackend, Flags>)(func);
            },
            [&]<typename Pred2 = Pred>()
            {
                return (RecursivelyVisitStaticTypesMatchingPred<T, Pred2, LoopBackend, Flags>)(func); // Not forwarding `func` because the two functions can run in reverse.
            }
        );
    }
}
