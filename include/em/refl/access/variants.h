#pragma once

#include "em/meta/cvref.h"
#include "em/meta/lists.h"

#include <cstddef>
#include <type_traits>
#include <variant>

namespace em::Refl::Variants
{
    /* The variant protocol:
     *
     * Apparently you can implement the variant protocol like you do with tuples.
     * Specialize following:
     *     `std::variant_size<T>` for cvref-unqualified `T` (the const version is added automatically)
     *     `std::variant_alternative<I, T>` for cvref-unqualified `T` (the const version is added automatically)
     * Provide following member functions:
     *     std::size_t index() const; // return -1 is valueless_by_exception
     *     bool valueless_by_exception() const; // [optional]
     * Provide following ADL functions, for all cvref variants:
     *     auto &&get<IorT>(V) // copy cvref qualifiers from the variant, [optional] for types
     *     auto *get_if<IorT>(V *) // [optional] for types
     *     decltype(auto) visit(F, V...) // [optional] for more or less than 1 variant argument
     *
     * Here the functions/overloads marked `[optional]` is not something we're going to use internally, but it's still a good idea to provide them for your users.
     */

    template <typename T>
    concept Type = requires{std::variant_size<std::remove_cvref_t<T>>::value;};

    template <typename T>
    concept TypeUnqualified = Meta::cvref_unqualified<T> && Type<T>;


    namespace detail
    {
        using std::get;

        template <typename T, std::size_t I>
        using AltTypeCvref = decltype(get<I>(std::declval<T>()));
    }

    // Extracts the `I`th alternative type with the correct cvref-qualifiers if any (this isn't necessarily a reference, but often will be one).
    // This is the return value of `get<I>(std::declval<T &&>())`. The proper cvref-qualifiers can't be reliably obtained from `std::variant_alterantive`.
    // `std::variant_alternative<I, T>` returns the same type but without ref-qualifiers (but with cv-qualifiers).
    template <Type T, std::size_t I>
    using AlternativeTypeCvref = detail::AltTypeCvref<T, I>;
}
