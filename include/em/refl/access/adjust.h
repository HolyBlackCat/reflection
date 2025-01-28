#pragma once

#include "em/macros/portable/no_unique_address.h"
#include "em/macros/utils/forward.h"
#include "em/macros/utils/lift.h"
#include "em/macros/utils/returns.h"
#include "em/meta/concepts.h"
#include "em/meta/constants.h"
#include "em/meta/cvref.h"
#include "em/meta/deduce.h"
#include "em/meta/packs.h"
#include "em/meta/void.h"
#include "em/refl/common.h"

#include <functional>
#include <type_traits>
#include <utility>

namespace em::Refl::Adjust
{
    namespace detail
    {
        // We could do `nonref_to_const_rvalue_ref<T>` here instead, but that doesn't seem to be actually useful.
        template <typename T>
        using MinimalAdjustment = T &&;

        template <typename T, template <typename> typename Pred>
        struct AdjustType
        {
            static constexpr bool adjusted = false;

            using type = MinimalAdjustment<T>;
        };

        template <typename T, template <typename> typename Pred>
        requires requires{_adl_em_refl_ReflectAs(custom::AdlDummy{}, std::declval<T>());}
        struct AdjustType<T, Pred>
        {
            using adjusted_type = typename AdjustType<decltype(EM_FWD_NONREF_AS_CONST(_adl_em_refl_ReflectAs(custom::AdlDummy{}, std::declval<T>()))), Pred>::type;
            static constexpr bool adjusted = Pred<adjusted_type>::value;
            using type = std::conditional_t<adjusted, adjusted_type, MinimalAdjustment<T>>;
        };
    }

    // Returns true if the type `T` can be adjusted (provides `_adl_em_refl_ReflectAs(...)`).
    // If `Pred` is specified and returns false for ALL types in the adjustment chain (`T` itself isn't tested), then will return false too.
    template <typename T, template <typename> typename Pred = Meta::true_trait>
    concept NeedsAdjustment = detail::AdjustType<T, Pred>::adjusted;

    // Returns the type that would be produced by `Adjust()`. That is:
    // If the type is adjustable via `_adl_em_refl_ReflectAs(AdlDummy{}, value)`, calls that repeatedly.
    //   Note that if any of them returns by value, we immediately replace that with a const rvalue reference.
    // If there are no suitable callbacks, we return `T` as is (except if it's non-reference, we return `T &&`; we don't add const in this case though).
    // If `Pred` returns false for the final type, we back out to the previous type repeatedly, until one matches.
    //   The `T` itself isn't checked and can be returned even if the preducate returns false for it.
    // Note that passing `T` is same as passing `T &&`, like in `std::declval()`.
    template <typename T, template <typename> typename Pred = Meta::true_trait>
    using AdjustType = typename detail::AdjustType<T, Pred>::type;

    // Transforms `value` by repeatedly applying `_adl_em_refl_ReflectAs(AdlDummy{}, value)` as long as it compiles, then calls `func` with the result.
    //   If `_adl_em_refl_ReflectAs(...)` returns by value, then it automatically becomes a const rvalue reference (this applies to intermediate objects too).
    //   The return value of `func` is forwarded as is.
    // If `Pred` is specified, we only convert to types for which it returns true (only the final type must satisfy, intermediate types don't have to).
    //   The `T` itself isn't checked against the predicate.
    // If no conversions are possible, returns `value` as is.
    template <
        template <typename> typename Pred = Meta::true_trait,
        Meta::Deduce..., typename T, std::invocable<AdjustType<T, Pred>> F
    >
    [[nodiscard]] constexpr auto Adjust(T &&value, F &&func) -> std::invoke_result_t<F &&, AdjustType<T, Pred>>
    {
        if constexpr (NeedsAdjustment<T, Pred>)
            return (Adjust<Pred>)(EM_FWD_NONREF_AS_CONST(_adl_em_refl_ReflectAs(custom::AdlDummy{}, EM_FWD(value))), EM_FWD(func));
        else
            return std::invoke(EM_FWD(func), EM_FWD(value));
    }
}
