#pragma once

#include "em/refl/common_iteration.h"
#include "em/refl/common.h"
#include "em/refl/contains_type_static.h"
#include "em/refl/for_each_matching_type.h"
#include "em/refl/visit_members_static.h"

namespace em::Refl
{
    template <Meta::TypePredicate Pred, Meta::LoopBackendType LoopBackend = Meta::LoopSimple, IterationFlags Flags = {}, Meta::Deduce..., typename T, typename F>
    requires RecursivelyIterableInThisDirectionForPred<T, Pred, LoopBackend, Flags>
    constexpr decltype(auto) ForEachStaticElemMatchingPred(T &&input, F &&func);

    // Recursively tries to find all static element types matching `Pred` in `input` (as if by `TypeRecursivelyContainsPred`).
    // Calls `func(elem)` on every matching element.
    // If `LoopBackend` iterates in reverse, then uses post-order traversal, otherwise pre-order.
    // Causes a SFINAE error if it finds a type matching `Pred` in a range that's not backward-iterable and `LoopBackend` wants backward iteration.
    // This version doesn't take a root object, so it will never call `func()` on the root object.
    template <typename T, Meta::TypePredicate Pred, Meta::LoopBackendType LoopBackend = Meta::LoopSimple, IterationFlags Flags = IterationFlags::ignore_root, Meta::Deduce..., typename F>
    requires RecursivelyIterableInThisDirectionForPred<T, Pred, LoopBackend, Flags> && (bool(Flags & IterationFlags::ignore_root))
    constexpr decltype(auto) ForEachStaticElemMatchingPred(F &&func)
    {
        return (ForEachTypeMatchingPred<T, PredTypeRecursivelyContainsStaticPred<Pred, Flags | IterationFlags::ignore_root>, LoopBackend, Flags & ~IterationFlags::ignore_root>)(
            [&]<typename SubT>() -> decltype(auto)
            {
                return (VisitStaticMembers<SubT, LoopBackend>)(
                    [&](auto &&member) -> decltype(auto)
                    {
                        return (ForEachStaticElemMatchingPred<Pred, LoopBackend, Flags & ~IterationFlags::ignore_root>)(member, func);
                    }
                );
            }
        );
    }

    // This version can call `func()` on the root object if it matches the predicate and `IterationFlags::ignore_root` is not used.
    template <Meta::TypePredicate Pred, Meta::LoopBackendType LoopBackend /*= Meta::LoopSimple*/, IterationFlags Flags /*={}*/, Meta::Deduce..., typename T, typename F>
    requires RecursivelyIterableInThisDirectionForPred<T, Pred, LoopBackend, Flags>
    constexpr decltype(auto) ForEachStaticElemMatchingPred(T &&input, F &&func)
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
                    return func(EM_FWD(input));
                }
                else
                {
                    return Meta::NoElements<LoopBackend>();
                }
            },
            [&]<typename Pred2 = Pred> -> decltype(auto)
            {
                return (ForEachStaticElemMatchingPred<T, Pred2, LoopBackend, Flags | IterationFlags::ignore_root>)(func);
            }
        );
    }


    // Recursively tries to find all static instances of `Elem` in `input`, as if by `TypeRecursivelyContainsStaticElemCvref`, so cvref on `Elem` matters,
    //   and it should normally be a reference.
    // Calls `func(elem)` on every matching element.
    // If `LoopBackend` iterates in reverse, then uses post-order traversal, otherwise pre-order.
    // Causes a SFINAE error if it finds a type matching `Pred` in a range that's not backward-iterable and `LoopBackend` wants backward iteration.
    // This version doesn't take a root object, so it will never call `func()` on the root object.
    template <typename T, typename Elem, Meta::LoopBackendType LoopBackend = Meta::LoopSimple, IterationFlags Flags = IterationFlags::ignore_root, Meta::Deduce..., typename F>
    requires RecursivelyIterableInThisDirectionForTypeCvref<T, Elem, LoopBackend, Flags> && (bool(Flags & IterationFlags::ignore_root))
    constexpr decltype(auto) ForEachStaticElemOfTypeCvref(F &&func)
    {
        // Passing `predicate_finds_bases` has no effect here, but I'm still doing it for consistency with the non-static version.
        return (ForEachStaticElemMatchingPred<T, PredTypeMatchesElemCvref<Elem>, LoopBackend, Flags | IterationFlags::predicate_finds_bases>)(EM_FWD(func));
    }

    // This version can call `func()` on the root object if it matches the predicate and `IterationFlags::ignore_root` is not used.
    template <typename Elem, Meta::LoopBackendType LoopBackend = Meta::LoopSimple, IterationFlags Flags = {}, Meta::Deduce..., typename T, typename F>
    requires RecursivelyIterableInThisDirectionForTypeCvref<T, Elem, LoopBackend, Flags>
    constexpr decltype(auto) ForEachStaticElemOfTypeCvref(T &&input, F &&func)
    {
        // Passing `predicate_finds_bases` has no effect here, but I'm still doing it for consistency with the non-static version.
        return (ForEachStaticElemMatchingPred<PredTypeMatchesElemCvref<Elem>, LoopBackend, Flags | IterationFlags::predicate_finds_bases>)(EM_FWD(input), EM_FWD(func));
    }
}
