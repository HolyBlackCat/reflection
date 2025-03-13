#pragma once

#include "em/refl/access/adjust.h"
#include "em/refl/access/bases.h"
#include "em/refl/access/indirect.h"
#include "em/refl/access/ranges.h"
#include "em/refl/access/structs.h"
#include "em/refl/access/variants.h"

namespace em::Refl
{
    enum class Category
    {
        adjust,
        indirect,
        structure,
        range,
        variant,
        unknown,
    };

    // Determines the preferred customization point to access the object of type `T`. Cvref-qualifiers are ignored.
    // The order is more or less arbitrary, except that adjustment should be first, our struct macros should probably be immediately after that.
    template <typename T>
    constexpr Category classify_opt = []{
        if constexpr (Adjust::NeedsAdjustment<T>)
            return Category::adjust;
        else if constexpr (Structs::Type<T> || Bases::HasBases<T>)
            return Category::structure;
        else if constexpr (Indirect::Type<T>)
            return Category::indirect;
        else if constexpr (Ranges::Type<T> && !Ranges::ElementTypeSameAsSelf<T>)
            return Category::range;
        else if constexpr (Variants::Type<T>)
            return Category::variant;
        else
            return Category::unknown;
    }();

    template <typename T>
    requires(classify_opt<T> != Category::unknown)
    constexpr Category classify = classify_opt<T>;
}
