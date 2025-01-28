#pragma once

#include "em/macros/utils/forward.h"
#include "em/meta/deduce.h"
#include "em/meta/lists.h"
#include "em/meta/cvref.h"
#include "em/refl/common.h"

#include <tuple>
#include <type_traits>
#include <utility>

namespace em::Refl::Structs
{
    namespace detail
    {
        // Here we try to get traits from `_adl_em_refl_StructMacro()` if that's available (that's for internal use by our macro only),
        // and fall back to `_adl_em_refl_Struct()` otherwise (that's for user customizations).

        template <typename T>
        struct SelectTraits
        {
            template <typename U = T>
            using type = decltype(_adl_em_refl_Struct(custom::AdlDummy{}, std::declval<const std::remove_cvref_t<U> *>()));
        };

        template <typename T>
        requires requires{_adl_em_refl_StructMacro(custom::AdlDummy{}, std::declval<const std::remove_cvref_t<T> *>());}
        struct SelectTraits<T>
        {
            template <typename = T>
            using type = decltype(_adl_em_refl_StructMacro(custom::AdlDummy{}, std::declval<const std::remove_cvref_t<T> *>()));
        };

        template <typename T>
        using Traits = typename SelectTraits<T>::template type<T>;
    }


    // A cvref-unqualified struct type. Must either contain `EM_REFL` macro or implement the same customization points.
    template <typename T>
    concept Type = Meta::cvref_unqualified<T> && requires{typename detail::Traits<T>;};

    template <typename T>
    concept TypeMaybeCvref = Type<std::remove_cvref_t<T>>;

    // The number of data members in a struct.
    template <TypeMaybeCvref T>
    constexpr int num_members = detail::Traits<T>::num_members;

    template <typename T, int I>
    concept ValidMemberIndex = I >= 0 && I < num_members<T>;

    // Get `I`th member of a struct, forced const.
    template <int I, Meta::Deduce..., TypeMaybeCvref T> requires ValidMemberIndex<T, I>
    [[nodiscard]] constexpr auto &&GetMemberConst(T &&object)
    {
        return detail::Traits<T>::template GetMember<I>(Meta::make_const(std::forward<T>(object)));
    }

    // Get `I`th member of a struct, mutable if the input is mutable.
    template <int I, Meta::Deduce..., TypeMaybeCvref T> requires ValidMemberIndex<T, I>
    [[nodiscard]] constexpr auto &&GetMemberMutable(T &&object)
    {
        return detail::Traits<T>::template GetMember<I>(std::forward<T>(object));
    }


    namespace detail
    {
        // If `Traits::GetMemberInfo()` returns a struct with a `::type` member that's not void, return that exactly.
        // Otherwise `decltype` the result of `GetMemberMutable()`, and remove cvref-qualifiers from that.
        template <Type T, int I>
        struct MemberType {using type = std::remove_cvref_t<decltype(GetMemberMutable<I>(std::declval<T &>()))>;};
        template <Type T, int I> requires(!std::is_void_v<typename decltype(Traits<T>::template GetMemberInfo<I>())::type>)
        struct MemberType<T, I> {using type = typename decltype(Traits<T>::template GetMemberInfo<I>())::type;};
    }

    // Returns the type of the `I`th struct member. Cvref-qualifiers on `T` are ignored.
    template <TypeMaybeCvref T, int I> requires ValidMemberIndex<T, I>
    using MemberType = typename detail::MemberType<std::remove_cvref_t<T>, I>::type;


    namespace detail
    {
        // If `_adl_em_refl_StructGetMemberInfo()` returns a struct with an `::attrs` member, extract template arguments from that into a `Meta::TypeList<...>`.
        // Otherwise returns an empty `Meta::TypeList<>`.
        template <Type T, int I>
        struct MemberAttributes {using type = Meta::TypeList<>;};
        template <Type T, int I> requires requires{typename decltype(Traits<T>::template GetMemberInfo<I>())::attrs;}
        struct MemberAttributes<T, I> {using type = Meta::list_from<typename decltype(Traits<T>::template GetMemberInfo<I>())::attrs>;};
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


    // This recognizes the tuple-like classes so we can provide an implementation for them.
    template <typename T>
    concept DefaultTupleLike = Meta::cvref_unqualified<T> && requires{std::tuple_size<T>::value;}; // `std::tuple_size_v` is not SFINAE-friendly.

    namespace detail
    {
        template <typename T, typename L>
        struct TupleMemberInfo {};

        template <typename T, std::size_t ...I>
        struct TupleMemberInfo<T, std::index_sequence<I...>> {using type = Meta::TypeList<MemberInfo<std::tuple_element_t<I, T>>...>;};
    }

    template <DefaultTupleLike T>
    struct DefaultTupleLikeTraits
    {
        static constexpr int num_members = int(std::tuple_size_v<T>);

        template <int I>
        [[nodiscard]] static constexpr auto &&GetMember(auto &&self) \
        {
            using std::get;
            return get<I>(EM_FWD(self));
        }

        template <int I>
        [[nodiscard]] static constexpr auto GetMemberInfo()
        {
            using List = typename detail::TupleMemberInfo<T, std::make_index_sequence<std::tuple_size_v<T>>>::type;
            return ::em::Meta::list_type_at<List, I>{};
        }
    };
}

namespace em::Refl::custom
{
    template <Structs::DefaultTupleLike T>
    constexpr auto _adl_em_refl_StructMacro(int/*AdlDummy*/, const T *)
    {
        return Structs::DefaultTupleLikeTraits<T>{};
    }
}
