#pragma once

#include "em/meta/cvref.h"
#include "em/refl/classify.h"

#include <type_traits>

namespace em::Refl
{
    namespace detail::ContainsType
    {
        // This uses a cycle: Check -> Check2 -> Check3 -> Check
        // `Check` applies some type transformations (non-reference to rvalue reference) and calls `Check2`.
        // `Check2` checks the predicate, and if it returns false, calls `Catch3`.
        // `Check3` has category-specific behavior. It runs `Check` for every member of the type. It tries to short-circuit when possible.

        template <typename T, template <typename> typename Pred, Category C>
        struct Check3 {};

        template <typename T, template <typename> typename Pred>
        struct Check2 : Check3<T, Pred, classify_opt<T>> {};
        template <typename T, template <typename> typename Pred> requires Pred<T>::value
        struct Check2<T, Pred> : std::true_type {};

        template <typename T, template <typename> typename Pred>
        using Check = Check2<T &&, Pred>;


        // Runs the predicate on all types in the list `T`. Copies the cvref-qualifiers from `Cvref` to each element before checking it.
        template <typename Cvref, typename T, template <typename> typename Pred>
        struct CheckTypeList {};
        template <typename Cvref, typename T0, typename ...T, template <typename> typename Pred>
        struct CheckTypeList<Cvref, Meta::TypeList<T0, T...>, Pred> : CheckTypeList<Cvref, Meta::TypeList<T...>, Pred> {};
        template <typename Cvref, typename T0, typename ...T, template <typename> typename Pred> requires Check<Meta::copy_cvref<Cvref, T0>, Pred>::value
        struct CheckTypeList<Cvref, Meta::TypeList<T0, T...>, Pred> : std::true_type {};

        // Checks all members of type `T`.
        template <typename T, template <typename> typename Pred, typename I = std::make_integer_sequence<int, Structs::num_members<T>>>
        struct CheckMembers {};
        template <typename T, template <typename> typename Pred>
        struct CheckMembers<T, Pred, std::integer_sequence<int>> : std::false_type {};
        template <typename T, template <typename> typename Pred, int I0, int ...I>
        struct CheckMembers<T, Pred, std::integer_sequence<int, I0, I...>> : CheckMembers<T, Pred, std::integer_sequence<int, I...>> {};
        template <typename T, template <typename> typename Pred, int I0, int ...I> requires Check<Structs::MemberTypeCvref<T, I0>, Pred>::value
        struct CheckMembers<T, Pred, std::integer_sequence<int, I0, I...>> : std::true_type {};

        // First checks the members of type `T`, then checks it bases.
        template <typename T, template <typename> typename Pred>
        struct CheckMembersAndBases : CheckTypeList<T, Bases::AllBasesFlat<T>, Pred> {};
        template <typename T, template <typename> typename Pred> requires CheckMembers<T, Pred>::value
        struct CheckMembersAndBases<T, Pred> : std::true_type {};

        template <typename T, template <typename> typename Pred, typename I = std::make_integer_sequence<int, std::variant_size_v<std::remove_cvref_t<T>>>>
        struct CheckVarAlts {};
        template <typename T, template <typename> typename Pred>
        struct CheckVarAlts<T, Pred, std::integer_sequence<int>> : std::false_type {};
        template <typename T, template <typename> typename Pred, int I0, int ...I>
        struct CheckVarAlts<T, Pred, std::integer_sequence<int, I0, I...>> : CheckVarAlts<T, Pred, std::integer_sequence<int, I...>> {};
        template <typename T, template <typename> typename Pred, int I0, int ...I> requires Check<Variants::AlternativeTypeCvref<T, I0>, Pred>::value
        struct CheckVarAlts<T, Pred, std::integer_sequence<int, I0, I...>> : std::true_type {};

        template <typename T, template <typename> typename Pred> struct Check3<T, Pred, Category::adjust> : Check<Adjust::AdjustedType<T>, Pred> {};
        template <typename T, template <typename> typename Pred> struct Check3<T, Pred, Category::indirect> : Check<Indirect::ValueType<T>, Pred> {};
        template <typename T, template <typename> typename Pred> struct Check3<T, Pred, Category::structure> : CheckMembersAndBases<T, Pred> {};
        template <typename T, template <typename> typename Pred> struct Check3<T, Pred, Category::range> : Check<Ranges::ElementTypeCvref<T>, Pred> {};
        template <typename T, template <typename> typename Pred> struct Check3<T, Pred, Category::variant> : CheckVarAlts<T, Pred> {};
        template <typename T, template <typename> typename Pred> struct Check3<T, Pred, Category::unknown> : std::false_type {};

        // ---

        template <typename T>
        struct PredSameIngnoringCvrefAndConvertible
        {
            template <typename U>
            using Pred = std::bool_constant<Meta::same_ignoring_cvref_and_convertible<U, T>>;
        };
    }

    // Checks if the type `T` contains an element of type `Elem` somewhere in it.
    // If `T` is a non-reference, it's assumed to be an rvalue reference, as in `std::declval<T>()`.
    // `Elem` is usually a reference. The tested types (which are always references) must match `Elem` ignoring cvref-qualifiers,
    //   and must be convertible to `Elem` (which normally only compares qualifiers, unless `Elem` is a non-reference, in which case it can check copy/move constructors).
    template <typename T, typename Elem>
    concept TypeContainsElemCvref = detail::ContainsType::Check<T, detail::ContainsType::PredSameIngnoringCvrefAndConvertible<Elem>::template Pred>::value;

    // Checks if the type `T` contains an element for which `Pred<__>::value` returns true.
    // The predicate should only receive references.
    // If `T` is a non-reference, `&&` is added automatically.
    template <typename T, template <typename> typename Pred>
    concept TypeContainsPred = detail::ContainsType::Check<T, Pred>::value;
}
