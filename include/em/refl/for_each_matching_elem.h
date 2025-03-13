#pragma once

#include "em/macros/platform/compiler.h"
#include "em/meta/const_for.h"
#include "em/meta/deduce.h"
#include "em/refl/common.h"
#include "em/refl/contains_type.h"
#include "em/refl/is_backward_iterable.h"
#include "em/refl/visit_members.h"

#include <type_traits>

namespace em::Refl
{
    // Whether we can recursively iterate over `T` in the direction specified by `LoopBackend`, in search for types matching `Pred`.
    // This can be false if `LoopBackend` wants backward iteration and there's a range somewhere in `T` that doesn't support it,
    //   AND this range recursively contains a type that matches `Pred` (because otherwise we don't need to iterate over it).
    template <typename T, typename/*TypePredicate*/ Pred, typename LoopBackend, IterationFlags Flags>
    concept RecursivelyIterableInThisDirectionForPred = !LoopBackend::is_reverse || bool(Flags & IterationFlags::fallback_to_not_reverse) || RecursivelyBackwardIterable<T, Pred>;

    // Whether we can recursively iterate over `T` in the direction specified by `LoopBackend`, in search for objects matching `Elem` (`Elem` normally is a reference).
    // This can be false if `LoopBackend` wants backward iteration and there's a range somewhere in `T` that doesn't support it,
    //   AND this range recursively contains a type that matches `Pred` (because otherwise we don't need to iterate over it).
    template <typename T, typename Elem, typename LoopBackend, IterationFlags Flags>
    concept RecursivelyIterableInThisDirectionForTypeCvref = RecursivelyIterableInThisDirectionForPred<T, PredTypeRecursivelyContainsElemCvref<Elem>, LoopBackend, Flags>;

    // Recursively tries to find all types matching `Pred` in `input` (as if by `TypeRecursivelyContainsPred`).
    // Calls `func(elem)` on every matching element.
    // If `LoopBackend` iterates in reverse, then uses post-order traversal, otherwise pre-order.
    // Causes a SFINAE error if it finds a type matching `Pred` in a range that's not backward-iterable and `LoopBackend` wants backward iteration.
    template <Meta::TypePredicate Pred, Meta::LoopBackendType LoopBackend = Meta::LoopSimple, IterationFlags Flags = {}, VisitMode Mode = VisitMode::normal, Meta::Deduce..., typename T, typename F>
    requires RecursivelyIterableInThisDirectionForPred<T, Pred, LoopBackend, Flags>
    constexpr decltype(auto) ForEachElemMatchingPred(T &&input, F &&func)
    {
        return Meta::RunEachFunc<LoopBackend>(
            [&]<typename Pred2 = Pred> -> decltype(auto)
            {
                if constexpr (Pred2::template type<T &&>::value)
                {
                    // Work around Clang bug: https://github.com/llvm/llvm-project/issues/61426
                    // Don't be confused by the ticket name, it apparently applies here.
                    // This isn't fixed in trunk yet at the time of writing, so the version condition might need to be bumped.
                    #if EM_IS_CLANG_VERSION(<= 20)
                    if constexpr (Pred::template type<T &&>::value)
                    #endif
                    return func(EM_FWD(input));
                }
                else
                {
                    return Meta::NoElements<LoopBackend>();
                }
            },
            [&]<typename Pred2 = Pred> -> decltype(auto)
            {
                if constexpr (TypeRecursivelyContainsPred<T, Pred2>)
                {
                    return (VisitMembers<LoopBackend, Flags, Mode>)(EM_FWD(input), [&]<typename Desc, VisitMode Submode>(auto &&member) -> decltype(auto)
                    {
                        return (ForEachElemMatchingPred<Pred2, LoopBackend, Flags, Submode>)(EM_FWD(member), EM_FWD(func));
                    });
                }
                else
                {
                    return Meta::NoElements<LoopBackend>();
                }
            }
        );
    }

    // Recursively tries to find all instances of `Elem` in `input`, as if by `TypeRecursivelyContainsElemCvref`, so cvref on `Elem` matters,
    //   and it should normally be a reference.
    // Calls `func(elem)` on every matching element.
    // If `LoopBackend` iterates in reverse, then uses post-order traversal, otherwise pre-order.
    // Causes a SFINAE error if it finds a type matching `Pred` in a range that's not backward-iterable and `LoopBackend` wants backward iteration.
    template <typename Elem, Meta::LoopBackendType LoopBackend = Meta::LoopSimple, IterationFlags Flags = {}, VisitMode Mode = VisitMode::normal, Meta::Deduce..., typename T, typename F>
    requires RecursivelyIterableInThisDirectionForTypeCvref<T, Elem, LoopBackend, Flags>
    constexpr decltype(auto) ForEachElemOfTypeCvref(T &&input, F &&func)
    {
        return (ForEachElemMatchingPred<PredTypeMatchesElemCvref<Elem>, LoopBackend, Flags, Mode>)(EM_FWD(input), EM_FWD(func));
    }
}
