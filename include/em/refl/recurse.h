#pragma once

#include "em/refl/access/adjust.h"
#include "em/refl/access/indirect.h"
#include "em/refl/access/ranges.h"
#include "em/refl/access/structs.h"

#include <type_traits>

namespace em::Refl::Recurse
{
    namespace detail
    {
        template <typename T, typename T>
        struct SameIgnoringCvref


        template <typename T>
        struct SpecificType
        {
            template <typename U>
            struct Pred : std::bool_constant<em::Meta::same_ignoring_cvref<>> {};
        };

        template <typename T>
        concept CanAdjustToSpecificType = Adjust::NeedsAdjustment<T, SpecificType<T>::template Pred>;
    }

    template <typename
}
