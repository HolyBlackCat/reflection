#pragma once

#include "em/macros/platform/compiler.h"
#include "em/meta/const_for.h"
#include "em/refl/common.h"
#include "em/refl/contains_type.h"
#include "em/refl/visit_types.h"

#include <concepts>

namespace em::Refl
{
    // Recursively tries to find all types of non-static members matching `Pred` in `T` (as if by `TypeRecursivelyContainsPred`). If `T` is a non-reference, adds `&&` automatically.
    // Calls `func<Type>()` on every matching element. The `Type` argument is always a reference.
    // If `LoopBackend` iterates in reverse, then uses post-order traversal, otherwise pre-order.
    template <typename T, Meta::TypePredicate Pred, Meta::LoopBackendType LoopBackend = Meta::LoopSimple, IterationFlags Flags = {}, VisitMode Mode = VisitMode::normal, Meta::Deduce..., typename F>
    constexpr decltype(auto) RecursivelyVisitTypesMatchingPred(F &&func)
    {
        static constexpr bool is_new_instance = !(Mode == VisitMode::base_subobject && bool(Flags & IterationFlags::predicate_finds_bases));

        static constexpr IterationFlags next_flags = is_new_instance ? Flags & ~IterationFlags::ignore_root : Flags;

        static constexpr IterationFlags next_flags_base = []{
            if constexpr (bool(Flags & IterationFlags::predicate_finds_bases) && !bool(Flags & IterationFlags::ignore_root))
                return next_flags | IterationFlags::ignore_root * Pred::template type<T &&>::value;
            else
                return next_flags;
        }();

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
                // Here `TypeRecursivelyContainsPred` doesn't respect `ignore_root` (returns a false positive),
                //   but since the resulting iteration only traverses bases, it should be free.
                // It's easier to do this than to patch `TypeRecursivelyContainsPred`.
                // Also we don't REALLY need to check the predicate here (compared to e.g. member iteration, since iterating over containers might not get optimized out,
                //   and also might not compile if iterating backwards, for some containers),
                //   but I'm leaving it here to hopefully make things faster. Not sure if it makes sense.
                if constexpr (TypeRecursivelyContainsPred<T, Pred2>)
                {
                    return (VisitTypes<T, LoopBackend, Mode>)([&]<typename SubT, VisitDesc Desc> -> decltype(auto)
                    {
                        static constexpr IterationFlags cur_flags = std::derived_from<VisitingAnyBase, Desc> ? next_flags_base : next_flags;

                        return (RecursivelyVisitTypesMatchingPred<SubT, Pred2, LoopBackend, cur_flags, Desc::mode>)(func);
                    });
                }
                else
                {
                    return Meta::NoElements<LoopBackend>();
                }
            }
        );
    }
}
