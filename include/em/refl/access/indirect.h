#pragma once

#include "em/macros/utils/forward.h"
#include "em/meta/concepts.h"
#include "em/meta/cvref.h"
#include "em/meta/deduce.h"
#include "em/refl/common.h"

#include <type_traits>
#include <utility>

// This file handles optionals, pointers (smart and dumb), etc.
// It also happens non-nullable indirect types, like iterators.
//   (For iterators, apparently default-constructed iterators are required to compare equal to each other, but they are not required to compare
//   unequal to other iterators. Even though I think that should always be the case in practice for forward iterators or stronger (e.g. it's not the case
//   for `std::istream_iterator`s, which are input iterators), I'm not doing that right now, just in case.)

namespace em::Refl::Indirect
{
    namespace detail
    {
        template <typename T>
        using Traits = decltype(_adl_em_refl_Indirect(custom::AdlDummy{}, std::declval<const std::remove_cvref_t<T> *>()));
    }


    template <typename T>
    concept Type = requires{typename detail::Traits<std::remove_cvref_t<T>>;};

    template <typename T>
    concept TypeUnqualified = Meta::cvref_unqualified<T> && Type<T>;

    // Does the object hold a value?
    // This will always return true if `AlwaysHasValue<T>` is true.
    template <Meta::Deduce..., TypeUnqualified T>
    [[nodiscard]] constexpr bool HasValue(const T &value)
    {
        return detail::Traits<T>::HasValue(value);
    }

    // Will `HasValue()` always return true for this type?
    template <typename T>
    concept AlwaysHasValue =
        Type<T> &&
        std::is_same_v<decltype(detail::Traits<T>::HasValue(std::declval<const std::remove_cvref_t<T> &>())), std::true_type>;

    // The result of calling `GetValue()`. Can depend on cvref-qualifiers on `T`. Usually a reference, but doesn't have to be one.
    template <Type T>
    using ValueType = decltype(detail::Traits<T>::GetValue(std::declval<T>()));

    // Get the value of an optional. This can perfect-forward.
    // This will usually return a reference, but I guess there's no reason why a customized optional can't return by value here.
    template <Meta::Deduce..., Type T>
    [[nodiscard]] constexpr ValueType<T> GetValue(T &&value)
    {
        return detail::Traits<T>::GetValue(std::forward<T>(value));
    }


    // Our default optional concept.
    template <typename T>
    concept DefaultCondition = Meta::cvref_unqualified<T> && requires(const T &t){*t;};

    template <typename T>
    struct DefaultTraits
    {
        [[nodiscard]] static constexpr auto HasValue(const T &value)
        {
            if constexpr (Meta::bool_convertible<T>)
                return bool(value);
            else
                return std::true_type{};
        }

        [[nodiscard]] static constexpr decltype(auto) GetValue(Meta::same_ignoring_cvref<T> auto &&value)
        {
            return *EM_FWD(value);
        }
    };
}

namespace em::Refl::custom
{
    // Does the optional have a value?
    template <Indirect::DefaultCondition T>
    constexpr auto _adl_em_refl_Indirect(int/*AdlDummy*/, const T *)
    {
        return Indirect::DefaultTraits<T>{};
    }
}
