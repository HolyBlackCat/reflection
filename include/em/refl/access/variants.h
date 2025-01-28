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
    concept Type = Meta::cvref_unqualified<T> && requires{std::variant_size<T>::value;};

    template <typename T>
    concept TypeMaybeCvref = Type<std::remove_cvref_t<T>>;


    namespace detail
    {
        template <typename T, typename I>
        struct Alternatives {};

        template <typename T, std::size_t ...I>
        struct Alternatives<T, std::index_sequence<I...>> {using type = Meta::TypeList<std::variant_alternative_t<I, T>...>;};
    }

    // A `Meta::TypeList<...>` of the types in a variant.
    // This follows the variant protocol instead of directly looking at the template parameters, so it can work for your own types too.
    template <TypeMaybeCvref T>
    using Alternatives = typename detail::Alternatives<std::remove_cvref_t<T>, std::make_index_sequence<std::variant_size_v<std::remove_cvref_t<T>>>>::type;
}
