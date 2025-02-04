#pragma once

#include "em/refl/common.h"

#include <utility>

namespace em::Refl::Adjust
{
    // Returns true if the type `T` can be adjusted (provides `_adl_em_refl_ReflectAs()`).
    template <typename T>
    concept NeedsAdjustment = requires{_adl_em_refl_ReflectAs(custom::AdlDummy{}, std::declval<T>());};

    // Returns the type that would be produced by `Adjust()`. If `T` isn't a reference, assumes `&&`, like `std::declval`.
    // Usually returns a reference, but not always.
    template <NeedsAdjustment T>
    using AdjustedType = decltype(_adl_em_refl_ReflectAs(custom::AdlDummy{}, std::declval<T>()));

    // Adjusts the object by calling `_adl_em_refl_ReflectAs()` if supported, otherwise does nothing and returns the argument as is.
    // Usually returns by reference (including if no callback). The callback can sometimes return by value.
    template <typename T>
    [[nodiscard]] constexpr decltype(auto) Adjust(T &&value)
    {
        if constexpr (NeedsAdjustment<T>)
            return _adl_em_refl_ReflectAs(custom::AdlDummy{}, std::forward<T>(value));
        else
            return std::forward<T>(value);
    }
}
