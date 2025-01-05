#pragma once

#include "em/meta/deduce.h"
#include "em/meta/lists.h"
#include "em/meta/qualifiers.h"
#include "em/refl/common.h"

#include <type_traits>
#include <utility>

namespace em::Refl::Struct
{
    // A cvref-unqualified struct type. Must either contain `EM_REFL` macro or implement the same customization points.
    template <typename T>
    concept Type = Meta::cvref_unqualified<T> && requires(const T *t){_adl_em_refl_NumMembers(custom::AdlDummy{}, t);};

    template <typename T>
    concept TypeMaybeCvref = Type<std::remove_cvref_t<T>>;

    // The number of data members in a struct.
    template <TypeMaybeCvref T>
    constexpr int num_members = _adl_em_refl_NumMembers(custom::AdlDummy{}, (const std::remove_cvref_t<T> *)nullptr);

    template <typename T, int I>
    concept ValidMemberIndex = I >= 0 && I < num_members<T>;

    // Get `I`th member of a struct, forced const.
    template <int I, Meta::Deduce..., TypeMaybeCvref T> requires ValidMemberIndex<T, I>
    [[nodiscard]] constexpr auto &&GetMemberConst(T &&object)
    {
        return _adl_em_refl_GetMember<I>(custom::AdlDummy{}, Meta::make_const(std::forward<T>(object)));
    }

    // Get `I`th member of a struct, mutable if the input is mutable.
    template <int I, Meta::Deduce..., TypeMaybeCvref T> requires ValidMemberIndex<T, I>
    [[nodiscard]] constexpr auto &&GetMemberMutable(T &&object)
    {
        return _adl_em_refl_GetMember<I>(custom::AdlDummy{}, std::forward<T>(object));
    }


    namespace detail
    {
        // If `_adl_em_refl_GetMemberInfo()` returns a struct with a `::type` member that's not void, return that exactly.
        // Otherwise `decltype` the result of `GetMemberMutable()`, and remove cvref-qualifiers from that.
        template <Type T, int I>
        struct MemberType {using type = std::remove_cvref_t<decltype(GetMemberMutable<I>(std::declval<T &>()))>;};
        template <Type T, int I> requires(!std::is_void_v<typename decltype(_adl_em_refl_GetMemberInfo<I>(custom::AdlDummy{}, (const std::remove_cvref_t<T> *)nullptr))::type>)
        struct MemberType<T, I> {using type = typename decltype(_adl_em_refl_GetMemberInfo<I>(custom::AdlDummy{}, (const std::remove_cvref_t<T> *)nullptr))::type;};
    }

    // Returns the type of the `I`th struct member. Cvref-qualifiers on `T` are ignored.
    template <TypeMaybeCvref T, int I> requires ValidMemberIndex<T, I>
    using MemberType = typename detail::MemberType<std::remove_cvref_t<T>, I>::type;


    namespace detail
    {
        // If `_adl_em_refl_GetMemberInfo()` returns a struct with an `::attrs` member, return that exactly.
        // Otherwise returns an empty `TypeList<>`.
        template <Type T, int I>
        struct MemberAttributes {using type = Meta::TypeList<>;};
        template <Type T, int I> requires requires{typename decltype(_adl_em_refl_GetMemberInfo<I>(custom::AdlDummy{}, (const std::remove_cvref_t<T> *)nullptr))::attrs;}
        struct MemberAttributes<T, I> {using type = typename decltype(_adl_em_refl_GetMemberInfo<I>(custom::AdlDummy{}, (const std::remove_cvref_t<T> *)nullptr))::attrs;};
    }

    // Returns the `TypeList<...>` of attributes of the `I`th struct member. Cvref-qualifiers on `T` are ignored.
    // If no attributes are specified, this is empty.
    template <TypeMaybeCvref T, int I> requires ValidMemberIndex<T, I>
    using MemberAttributes = typename detail::MemberAttributes<std::remove_cvref_t<T>, I>::type;


    namespace detail
    {
        template <typename Base, typename ...A>
        struct FindAttributeLow {};

        template <typename Base, typename A0, typename ...A>
        struct FindAttributeLow<Base, A0, A...> : FindAttributeLow<Base, A...> {};

        template <typename Base, typename A0, typename ...A> requires std::is_base_of_v<Base, A0>
        struct FindAttributeLow<Base, A0, A...>
        {
            using type = A0;
            static_assert((!std::is_base_of_v<Base, A> && ...), "More than one matchiing attribute in the list.");
        };

        template <typename Base, typename List>
        struct FindAttribute {};

        template <typename Base, typename ...A>
        struct FindAttribute<Base, Meta::TypeList<A...>> : FindAttributeLow<Base, A...> {};
    }

    // Inspects the list of attributes of a member (`MemberAttributes<T, I>`) to find exactly `A` or a class inherited from it.
    // On success returns the found type. On failure triggers a SFINAE error. If more than one attribute matches, triggers a hard error.
    template <TypeMaybeCvref T, int I, AttributeOrBase A> requires ValidMemberIndex<T, I>
    using MemberFindAttribute = typename detail::FindAttribute<A, MemberAttributes<T, I>>::type;

    // Returns true if `I`th member variable of `T` has attribute `A` or an attribute inherited from it.
    // False if no such attribute. Hard error if more than one such attribute.
    template <TypeMaybeCvref T, int I, AttributeOrBase A> requires ValidMemberIndex<T, I>
    constexpr bool member_has_attribute = requires{typename MemberFindAttribute<T, I, A>;};
}
