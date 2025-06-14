#pragma once

#include "em/meta/const_for.h"
#include "em/meta/common.h"
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

    // The range element type. Cvref-qualifiers of `T` are ignored.
    template <Type T>
    using ElementType = std::ranges::range_value_t<std::remove_cvref_t<T>>;

    // Whether `T` is a range with the element type also being `T`. Cvref-qualifiers on `T` are ignored.
    // This is true for silly types like `std::filesystem::path`.
    template <typename T>
    concept ElementTypeSameAsSelf = Meta::same_ignoring_cvref<T, ElementType<T>>;

    // A heuristic to check that a range owns its elements. If true, when the range is an rvalue, we should forward the elements.
    // Note that the forwarding should use `std::ranges::iter_move()` and friends.
    // Cvref-qualifiers on `T` are ignored here. You must manually check that it's an rvalue before moving the elements.
    template <typename T>
    concept ShouldForwardElements = Type<T> && _adl_em_refl_RangeShouldForwardElements(custom::AdlDummy{}, (const std::remove_cvref_t<T> *)nullptr);


    namespace detail
    {
        template <typename T>
        struct RangeForwardedRef {using type = std::ranges::range_reference_t<std::remove_reference_t<T>>;};
        template <typename T> requires std::is_rvalue_reference_v<T &&> && ShouldForwardElements<T>
        struct RangeForwardedRef<T> {using type = std::ranges::range_rvalue_reference_t<std::remove_reference_t<T>>;};
    }

    // An appropriate reference to a range element.
    // If `ShouldForwardElements<T> == true`, this is forced to be an rvalue reference (or something that behaves like one, e.g. a tuple of rvalue references).
    template <Type T>
    using ElementTypeCvref = typename detail::RangeForwardedRef<T>::type;

    // Forwards the element of a range `T` based on cvref-qualifiers of `T`.
    template <Type T, Meta::Deduce..., typename E>
    [[nodiscard]] constexpr ElementTypeCvref<T> ForwardElement(E &&elem)
    {
        return static_cast<ElementTypeCvref<T>>(elem);
    }

    // Dereferences an iterator and forwards the result, for a range `T` based on cvref-qualifiers of `T`.
    template <Type T, Meta::Deduce..., typename I>
    [[nodiscard]] constexpr ElementTypeCvref<T> ForwardDerefIter(I &&iter)
    {
        if constexpr (std::is_rvalue_reference_v<T &&> && ShouldForwardElements<T>)
            return std::ranges::iter_move(std::forward<I>(iter));
        else
            return *std::forward<I>(iter);
    }


    // Can we iterate over this range backwards? Cvref-qualifiers on `T` are ignored.
    template <typename T>
    concept BackwardIterable = Type<T> && std::ranges::bidirectional_range<std::remove_cvref_t<T>>;
    // Like `BackwardIterable`, but returns true for non-ranges instead of false.
    template <typename T>
    concept BackwardIterableOrNonRange = !Type<T> || BackwardIterable<T>;

    // Iterates over a range.
    // Unlike the built-in loops, automatically forwards the elements correctly.
    // If `LoopBackend` wants to iterate in reverse, the range isn't iterable in reverse, and `IterationFlags::fallback_to_not_reverse` isn't set,
    //   a SFINAE error is generated.
    template <Meta::LoopBackendType LoopBackend, IterationFlags Flags = {}, Meta::Deduce..., typename T>
    requires (!LoopBackend::is_reverse) || (bool(Flags & IterationFlags::fallback_to_not_reverse)) || BackwardIterable<T>
    [[nodiscard]] decltype(auto) ForEach(T &&range, auto &&func)
    {
        if constexpr (LoopBackend::is_reverse && !BackwardIterable<T>)
        {
            // If we're trying to iterate backwards and the loop backend doesn't support it, fall back to the normal iteration.
            return (ForEach<typename LoopBackend::reverse, Flags>)(EM_FWD(range), EM_FWD(func));
        }
        else
        {
            return Meta::ForEach<LoopBackend>(std::ranges::begin(range), std::ranges::end(range), [&](auto &&elem) -> decltype(auto) {return func((ForwardElement<T>)(elem));});
        }
    }


    // The first element of a range. Uses `.front()` if present. Perfect-forwarded if applicable.
    template <Meta::Deduce..., Type T>
    [[nodiscard]] constexpr ElementTypeCvref<T> Front(T &&range)
    {
        if constexpr (requires{range.front();})
            return ForwardElement<T>(range.front());
        else
            return ForwardDerefIter<T>(std::ranges::begin(range));
    }
    // The last element of a range. Uses `.back()` if present, otherwise `*std::prev(end)` if present, otherwise causes a SFINAE error.
    template <Meta::Deduce..., Type T>
    [[nodiscard]] constexpr ElementTypeCvref<T> Back(T &&range)
    requires requires{range.back();} || std::ranges::bidirectional_range<std::remove_cvref_t<T>>
    {
        if constexpr (requires{range.back();})
            return ForwardElement<T>(range.back());
        else
            return ForwardDerefIter<T>(std::prev(std::ranges::end(range)));
    }
}

namespace em::Refl::custom
{
    // The default implementation of `ShouldForwardElements`.
    template <Meta::Deduce..., typename T>
    constexpr bool _adl_em_refl_RangeShouldForwardElements(int/*AdlDummy*/, const T *)
    {
        // First we check the value type for referenceness. If it is, we naturally shouldn't propagate rvalue-ness to them.
        if constexpr (!std::is_reference_v<std::ranges::range_value_t<std::remove_cvref_t<T>>>)
            return requires(T &t){t.erase(t.begin());}; // Then check that the range owns its elements.
        else
            return false;
    }
}
