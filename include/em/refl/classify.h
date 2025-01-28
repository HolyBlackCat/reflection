#pragma once

#include "em/meta/constants.h"
#include "em/refl/access/adjust.h"
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

    template <typename T, template <typename> typename Pred = Meta::true_trait>
    constexpr Category category_opt = []{
        if constexpr (Adjust::NeedsAdjustment<T, Pred>)
            return Category::adjust;
        else if constexpr (Indirect::Type<T>)
            return Category::indirect;
        else if constexpr (Ranges::Type<T>)
            return Category::range;
        else if constexpr (Structs::Type<T>)
            return Category::structure;
        else if constexpr (Variants::Type<T>)
            return Category::variant;
        else
            return Category::unknown;
    }();

    template <typename T, template <typename> typename Pred = Meta::true_trait>
    requires(category_opt<T> != Category::unknown)
    constexpr Category category = category_opt<T, Pred>;
}
