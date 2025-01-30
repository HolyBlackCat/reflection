#pragma once

#include "em/meta/deduce.h"
#include "em/meta/functional.h"
#include "em/meta/tags.h"
#include "em/refl/common.h"

#include <iterator>
#include <ranges>
#include <type_traits>

namespace em::Refl::Ranges
{
    // We're reusing the standard range abstraction, but provide some helper functions.

    // Our range concept.
    template <typename T>
    concept Type = std::ranges::forward_range<std::remove_cvref_t<T>>;

    template <typename T>
    concept TypeUnqualified = Meta::cvref_unqualified<T> && Type<T>;

    // A heuristic to check that a range owns its elements. If true, when the range is an rvalue, we should forward the elements.
    template <typename T>
    concept ShouldForwardElements = Type<T> && _adl_em_refl_RangeShouldForwardElements(custom::AdlDummy{}, (const std::remove_cvref_t<T> *)nullptr);

    // The range element type. Cvref-qualifiers of `T` are ignored.
    template <Type T>
    using ElementType = std::ranges::range_value_t<std::remove_cvref_t<T>>;

    // The range element type, with appropariate cvref-qualifiers added based on those of `T`.
    // Can only produce rvalues if `ShouldForwardElements<T> == true`.
    template <Type T>
    using ElementCvref = Meta::copy_cvref<std::conditional_t<ShouldForwardElements<T>, T &&, T &>, ElementType<T>>;

    // Forwards the element of a range `T` based on cvref-qualifiers of `T`.
    template <Type T, Meta::Deduce..., typename E>
    [[nodiscard]] constexpr ElementCvref<T> ForwardElement(E &&elem)
    {
        return static_cast<ElementCvref<T>>(elem);
    }

    // The first element of a range. Uses `.front()` if present. Perfect-forwarded if applicable.
    template <Meta::Deduce..., Type T>
    [[nodiscard]] constexpr ElementCvref<T> Front(T &&range)
    {
        if constexpr (requires{range.front();})
            return ForwardElement<T>(range.front());
        else
            return ForwardElement<T>(*std::ranges::begin(range));
    }
    // The last element of a range. Uses `.back()` if present, otherwise `*std::prev(end)` if present, otherwise causes a SFINAE error.
    template <Meta::Deduce..., Type T>
    [[nodiscard]] constexpr ElementCvref<T> Back(T &&range)
    requires requires{range.back();} || std::ranges::bidirectional_range<std::remove_cvref_t<T>>
    {
        if constexpr (requires{range.back();})
            return ForwardElement<T>(range.back());
        else
            return ForwardElement<T>(*std::prev(std::ranges::end(range)));
    }
}

namespace em::Refl::custom
{
    // The default implementation of `ShouldForwardElements`.
    template <Meta::Deduce..., typename T>
    constexpr bool _adl_em_refl_RangeShouldForwardElements(int/*AdlDummy*/, const T *)
    {
        if constexpr (!std::is_reference_v<std::ranges::range_value_t<std::remove_cvref_t<T>>>)
            return requires(T &t){t.erase(std::ranges::begin(t));};
        else
            return false;
    }
}
