#pragma once

#include "em/refl/common.h"
#include "em/refl/recursively_visit_elems.h"
#include "em/refl/recursively_visit_types.h"
#include "em/refl/visit_members_static.h"

namespace em::Refl
{
    template <Meta::TypePredicate Pred, Meta::LoopBackendType LoopBackend = Meta::LoopSimple, IterationFlags Flags = {}, Meta::Deduce..., typename T, typename F>
    constexpr decltype(auto) RecursivelyVisitStaticElemsMatchingPred(T &&input, F &&func);

    // Recursively tries to find all static element types matching `Pred` in `input` (as if by `TypeRecursivelyContainsPred`).
    // Calls `func(elem)` on every matching element.
    // If `LoopBackend` iterates in reverse, then uses post-order traversal, otherwise pre-order.1
    // Causes a SFINAE error if it finds a type matching `Pred` in a range that's not backward-iterable and `LoopBackend` wants backward iteration.
    // This version doesn't take a root object, so it will never call `func()` on the root object.
    // Ignores `ignore_root`, since this will never visit the root anyway.
    template <typename T, Meta::TypePredicate Pred, Meta::LoopBackendType LoopBackend = Meta::LoopSimple, IterationFlags Flags = IterationFlags::root_is_not_static, Meta::Deduce..., typename F>
    constexpr decltype(auto) RecursivelyVisitStaticElemsMatchingPred(F &&func)
    {
        // Not ignoring root, because in each type we'll only visit its elements, not the type itself, so we don't mind visiting the root type. This flag is only needed for the other overload (see below).
        // Marking the root subtree as non-static (in each type), because we won't be actually iterating over the non-static members of the matches. We're only interested in their static members. I really hope this is correct.
        return (RecursivelyVisitTypesMatchingPred<T, std::remove_cvref, PredTypeRecursivelyContainsStaticPred<Pred, Flags | IterationFlags::root_is_not_static>, LoopBackend, Flags & ~IterationFlags::ignore_root>)(
            [&]<typename SubT>() -> decltype(auto)
            {
                return (VisitStaticMembers<SubT, LoopBackend>)(
                    [&](auto &&member) -> decltype(auto)
                    {
                        // Marking those subtrees as static, for obvious reasons.
                        return (RecursivelyVisitStaticElemsMatchingPred<Pred, LoopBackend, Flags & ~IterationFlags::root_is_not_static>)(member, func);
                    }
                );
            }
        );
    }

    // This version can call `func()` on the root object if it matches the predicate and `IterationFlags::root_is_not_static` is not used.
    template <Meta::TypePredicate Pred, Meta::LoopBackendType LoopBackend /*= Meta::LoopSimple*/, IterationFlags Flags /*={}*/, Meta::Deduce..., typename T, typename F>
    constexpr decltype(auto) RecursivelyVisitStaticElemsMatchingPred(T &&input, F &&func)
    {
        return Meta::RunEachFunc<LoopBackend>(
            [&]<typename Pred2 = Pred> -> decltype(auto)
            {
                if (!bool(Flags & IterationFlags::root_is_not_static))
                    return (RecursivelyVisitElemsMatchingPred<Pred2, LoopBackend, Flags>)(EM_FWD(input), func);
            },
            [&]<typename Pred2 = Pred> -> decltype(auto)
            {
                return (RecursivelyVisitStaticElemsMatchingPred<T, Pred2, LoopBackend, Flags | IterationFlags::root_is_not_static>)(func);
            }
        );
    }


    // Recursively tries to find all static instances of `Elem` in `input`, as if by `TypeRecursivelyContainsStaticElemCvref`, so cvref on `Elem` matters,
    //   and it should normally be a reference.
    // Calls `func(elem)` on every matching element.
    // If `LoopBackend` iterates in reverse, then uses post-order traversal, otherwise pre-order.
    // Causes a SFINAE error if it finds a type matching `Pred` in a range that's not backward-iterable and `LoopBackend` wants backward iteration.
    // This version doesn't take a root object, so it will never call `func()` on the root object.
    template <typename T, typename Elem, Meta::LoopBackendType LoopBackend = Meta::LoopSimple, IterationFlags Flags = IterationFlags::root_is_not_static, Meta::Deduce..., typename F>
    constexpr decltype(auto) RecursivelyVisitStaticElemsOfTypeCvref(F &&func)
    {
        // Passing `predicate_finds_bases` has no effect here, but I'm still doing it for consistency with the non-static version.
        return (RecursivelyVisitStaticElemsMatchingPred<T, PredTypeMatchesElemCvref<Elem>, LoopBackend, Flags | IterationFlags::predicate_finds_bases>)(EM_FWD(func));
    }

    // This version can call `func()` on the root object if it matches the predicate and `IterationFlags::root_is_not_static` is not used.
    template <typename Elem, Meta::LoopBackendType LoopBackend = Meta::LoopSimple, IterationFlags Flags = {}, Meta::Deduce..., typename T, typename F>
    constexpr decltype(auto) RecursivelyVisitStaticElemsOfTypeCvref(T &&input, F &&func)
    {
        // Passing `predicate_finds_bases` has no effect here, but I'm still doing it for consistency with the non-static version.
        return (RecursivelyVisitStaticElemsMatchingPred<PredTypeMatchesElemCvref<Elem>, LoopBackend, Flags | IterationFlags::predicate_finds_bases>)(EM_FWD(input), EM_FWD(func));
    }
}
