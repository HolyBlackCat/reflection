#pragma once

#include "em/macros/platform/compiler.h"
#include "em/macros/utils/forward.h"
#include "em/meta/const_for.h"
#include "em/refl/common.h"
#include "em/refl/contains_type_static.h"
#include "em/refl/for_each_matching_type.h"
#include "em/refl/visit_types_static.h"

namespace em::Refl
{
    // Recursively tries to find all types of static members matching `Pred` in `T` (as if by `TypeRecursivelyContainsStaticPred`). If `T` is a non-reference, adds `&&` automatically.
    // Calls `func<Type>()` on every matching element. The `Type` argument is always a reference.
    // If `LoopBackend` iterates in reverse, then uses post-order traversal, otherwise pre-order.
    template <typename T, Meta::TypePredicate Pred, Meta::LoopBackendType LoopBackend = Meta::LoopSimple, IterationFlags Flags = {}, Meta::Deduce..., typename F>
    constexpr decltype(auto) ForEachStaticTypeMatchingPred(F &&func)
    {
        return Meta::RunEachFunc<LoopBackend>(
            [&]<typename Pred2 = Pred> -> decltype(auto)
            {
                if (bool(Flags & IterationFlags::ignore_root))
                {
                    return Meta::NoElements<LoopBackend>();
                }
                else if constexpr (Pred2::template type<T &&>::value)
                {
                    // Work around Clang bug: https://github.com/llvm/llvm-project/issues/61426
                    // Don't be confused by the ticket name, it apparently applies here.
                    // This isn't fixed in trunk yet at the time of writing, so the version condition might need to be bumped.
                    #if EM_IS_CLANG_VERSION(<= 21)
                    if constexpr (!bool(Flags & IterationFlags::ignore_root))
                    if constexpr (Pred::template type<T &&>::value) // Sic, stacking conditions.
                    #endif
                    return func.template operator()<T &&>();
                }
                else
                {
                    return Meta::NoElements<LoopBackend>();
                }
            },
            [&]<typename Pred2 = Pred> -> decltype(auto)
            {
                return (ForEachTypeMatchingPred<T, PredTypeRecursivelyContainsStaticPred<Pred, Flags | IterationFlags::ignore_root>, LoopBackend, Flags & ~IterationFlags::ignore_root>)(
                    [&]<typename SubT>() -> decltype(auto)
                    {
                        return (VisitTypesStatic<SubT, LoopBackend>)(
                            [&]<typename SubSubT> -> decltype(auto)
                            {
                                return (ForEachStaticTypeMatchingPred<SubSubT, Pred, LoopBackend, Flags & ~IterationFlags::ignore_root>)(EM_FWD(func));
                            }
                        );
                    }
                );
            }
        );
    }
}
