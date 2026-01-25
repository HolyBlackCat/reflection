#pragma once

#include "em/macros/utils/forward.h"
#include "em/refl/common_iteration.h"
#include "em/refl/recursively_visit_elems_static.h"
#include "em/refl/recursively_visit_elems.h"

namespace em::Refl
{
    // Combines visiting non-static and static elements.
    template <Meta::TypePredicate Pred, Meta::LoopBackendType LoopBackend /*= Meta::LoopSimple*/, IterationFlags Flags /*={}*/, Meta::Deduce..., typename T, typename F>
    requires RecursivelyIterableInThisDirectionForPred<T, Pred, LoopBackend, Flags>
    constexpr decltype(auto) RecursivelyVisitMaybeStaticElemsMatchingPred(T &&input, F &&func)
    {
        return Meta::RunEachFunc<LoopBackend>(
            [&]<typename Pred2 = Pred>()
            {
                return (RecursivelyVisitElemsMatchingPred<Pred2, LoopBackend, Flags>)(EM_FWD(input), func);
            },
            [&]<typename Pred2 = Pred>()
            {
                // Not using the version that accepts an object (`input`) because this object is assumed to be non-static, because we're also passing it to the non-static function above.
                //   And since we use this version of the function, we have to force `ignore_root` - it's in its SFINAE condition.
                // Not forwarding `func` because the two functions can run in reverse.
                return (RecursivelyVisitStaticElemsMatchingPred<T, Pred2, LoopBackend, Flags | IterationFlags::ignore_root>)(func);
            }
        );
    }

    // Combines visiting non-static and static elements.
    template <typename Elem, Meta::LoopBackendType LoopBackend = Meta::LoopSimple, IterationFlags Flags = {}, Meta::Deduce..., typename T, typename F>
    requires RecursivelyIterableInThisDirectionForTypeCvref<T, Elem, LoopBackend, Flags>
    constexpr decltype(auto) RecursivelyVisitMaybeStaticElemsOfTypeCvref(T &&input, F &&func)
    {
        return (RecursivelyVisitMaybeStaticElemsMatchingPred<PredTypeMatchesElemCvref<Elem>, LoopBackend, Flags>)(EM_FWD(input), EM_FWD(func));
    }
}
