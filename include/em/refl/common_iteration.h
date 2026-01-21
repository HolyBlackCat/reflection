#pragma once

#include "em/meta/type_predicates.h"
#include "em/refl/access/ranges.h"
#include "em/refl/common.h"
#include "em/refl/contains_type.h"

namespace em::Refl
{
    namespace detail::IsBackwardIterable
    {
        struct NonBackwardIterableRange
        {
            template <typename T>
            using type = std::bool_constant<!Ranges::BackwardIterableOrNonRange<T>>;
        };
    }

    // Checks all members recursively in `T`, returns false if any of them is a range that can't be iterated backwards.
    // Doesn't recurse into types for which `Filter` returns false, and ignores them completely even if they aren't backward-iterable.
    template <typename T, IterationFlags Flags = {}, typename/*TypePredicate*/ Filter = Meta::true_predicate>
    concept RecursivelyBackwardIterable = !TypeRecursivelyContainsPred<T, detail::IsBackwardIterable::NonBackwardIterableRange, Flags, Filter>;


    // Whether we can recursively iterate over `T` in the direction specified by `LoopBackend`, in search for types matching `Pred`.
    // This can be false if `LoopBackend` wants backward iteration and there's a range somewhere in `T` that doesn't support it,
    //   AND this range recursively contains a type that matches `Pred` (because otherwise we don't need to iterate over it).
    template <typename T, typename/*TypePredicate*/ Pred, typename LoopBackend, IterationFlags Flags>
    concept RecursivelyIterableInThisDirectionForPred = !LoopBackend::is_reverse || bool(Flags & IterationFlags::fallback_to_not_reverse) || RecursivelyBackwardIterable<T, Flags, Pred>;

    // Whether we can recursively iterate over `T` in the direction specified by `LoopBackend`, in search for objects matching `Elem` (`Elem` normally is a reference).
    // This can be false if `LoopBackend` wants backward iteration and there's a range somewhere in `T` that doesn't support it,
    //   AND this range recursively contains a type that matches `Pred` (because otherwise we don't need to iterate over it).
    template <typename T, typename Elem, typename LoopBackend, IterationFlags Flags>
    concept RecursivelyIterableInThisDirectionForTypeCvref = RecursivelyIterableInThisDirectionForPred<T, PredTypeRecursivelyContainsElemCvref<Elem>, LoopBackend, Flags>;
}
