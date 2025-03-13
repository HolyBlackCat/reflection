#pragma once

#include "em/refl/classify.h"
#include "em/meta/constants.h"
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
    template <typename T, typename/*TypePredicate*/ Filter = Meta::true_predicate>
    concept RecursivelyBackwardIterable = !TypeRecursivelyContainsPred<T, detail::IsBackwardIterable::NonBackwardIterableRange, Filter>;
}
