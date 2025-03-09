#pragma once

#include "em/meta/cvref.h"
#include "em/refl/classify.h"

#include <type_traits>

// `em::Refl::TypeContainsElemCvref` recursively checks that a type contains some other type.
// This could be implemented in terms of `VisitTypes` from `em/refl/visit_types.h`, but that has worse compilation times.

namespace em::Refl
{
    namespace detail::ContainsType
    {
        // This uses a cycle: Check -> Check2 -> Check3 -> Check
        // `Check` applies some type transformations (non-reference to rvalue reference) and calls `Check2`.
        // `Check2` checks the predicate, and if it returns false, calls `Catch3`.
        // `Check3` has category-specific behavior. It runs `Check` for every member of the type. It tries to short-circuit when possible.

        template <typename T, template <typename> typename Pred, VisitMode Mode, Category C>
        struct Check3 {};

        template <typename T, template <typename> typename Pred, VisitMode Mode>
        struct Check2 : Check3<T, Pred, Mode, classify_opt<T>> {};
        template <typename T, template <typename> typename Pred, VisitMode Mode> requires Pred<T>::value
        struct Check2<T, Pred, Mode> : std::true_type {};

        template <typename T, template <typename> typename Pred, VisitMode Mode = VisitMode::normal>
        using Check = Check2<T &&, Pred, Mode>;


        // Runs the predicate on all types in the list `T`. Copies the cvref-qualifiers from `Cvref` to each element before checking it.
        template <typename Cvref, typename T, template <typename> typename Pred>
        struct CheckBaseTypeListLow {};
        template <typename Cvref, typename T0, typename ...T, template <typename> typename Pred>
        struct CheckBaseTypeListLow<Cvref, Meta::TypeList<T0, T...>, Pred> : CheckBaseTypeListLow<Cvref, Meta::TypeList<T...>, Pred> {};
        template <typename Cvref, typename T0, typename ...T, template <typename> typename Pred> requires Check<Meta::copy_cvref<Cvref, T0>, Pred, VisitMode::base_subobject/*Note!*/>::value
        struct CheckBaseTypeListLow<Cvref, Meta::TypeList<T0, T...>, Pred> : std::true_type {};

        // Checks all members of type `T`.
        template <typename T, template <typename> typename Pred, typename I = std::make_integer_sequence<int, Structs::num_members<T>>>
        struct CheckMembers {};
        template <typename T, template <typename> typename Pred>
        struct CheckMembers<T, Pred, std::integer_sequence<int>> : std::false_type {};
        template <typename T, template <typename> typename Pred, int I0, int ...I>
        struct CheckMembers<T, Pred, std::integer_sequence<int, I0, I...>> : CheckMembers<T, Pred, std::integer_sequence<int, I...>> {};
        template <typename T, template <typename> typename Pred, int I0, int ...I> requires Check<Structs::MemberTypeCvref<T, I0>, Pred>::value
        struct CheckMembers<T, Pred, std::integer_sequence<int, I0, I...>> : std::true_type {};

        // Checks all bases of `T` if `Mode != base_subobject`, otherwise returns false.
        // NOTE! Unlike `VisitTypes`, here we iterate over a flat list of bases, instead of the proper hierarchy.
        // Primarily because the hierarchy is more expensive to compute, and it doesn't matter anyway. On the other hand, `VisitTypes` does respect the hierarchy.
        template <typename T, template <typename> typename Pred, VisitMode Mode>
        struct CheckBases : CheckBaseTypeListLow<T, Bases::AllBasesFlat<T>, Pred> {};
        template <typename T, template <typename> typename Pred>
        struct CheckBases<T, Pred, VisitMode::base_subobject> : std::false_type {};


        // First checks the members of type `T`, then checks it bases.
        template <typename T, template <typename> typename Pred, VisitMode Mode>
        struct CheckMembersAndBases : CheckBases<T, Pred, Mode> {};
        template <typename T, template <typename> typename Pred, VisitMode Mode> requires CheckMembers<T, Pred>::value
        struct CheckMembersAndBases<T, Pred, Mode> : std::true_type {};

        template <typename T, template <typename> typename Pred, typename I = std::make_integer_sequence<int, std::variant_size_v<std::remove_cvref_t<T>>>>
        struct CheckVarAlts {};
        template <typename T, template <typename> typename Pred>
        struct CheckVarAlts<T, Pred, std::integer_sequence<int>> : std::false_type {};
        template <typename T, template <typename> typename Pred, int I0, int ...I>
        struct CheckVarAlts<T, Pred, std::integer_sequence<int, I0, I...>> : CheckVarAlts<T, Pred, std::integer_sequence<int, I...>> {};
        template <typename T, template <typename> typename Pred, int I0, int ...I> requires Check<Variants::AlternativeTypeCvref<T, I0>, Pred>::value
        struct CheckVarAlts<T, Pred, std::integer_sequence<int, I0, I...>> : std::true_type {};

        template <typename T, template <typename> typename Pred, VisitMode Mode> struct Check3<T, Pred, Mode, Category::adjust> : Check<Adjust::AdjustedType<T>, Pred> {};
        template <typename T, template <typename> typename Pred, VisitMode Mode> struct Check3<T, Pred, Mode, Category::indirect> : Check<Indirect::ValueTypeCvref<T>, Pred> {};
        template <typename T, template <typename> typename Pred, VisitMode Mode> struct Check3<T, Pred, Mode, Category::structure> : CheckMembersAndBases<T, Pred, Mode> {};
        template <typename T, template <typename> typename Pred, VisitMode Mode> struct Check3<T, Pred, Mode, Category::range> : Check<Ranges::ElementTypeCvref<T>, Pred> {};
        template <typename T, template <typename> typename Pred, VisitMode Mode> struct Check3<T, Pred, Mode, Category::variant> : CheckVarAlts<T, Pred> {};
        template <typename T, template <typename> typename Pred, VisitMode Mode> struct Check3<T, Pred, Mode, Category::unknown> : std::false_type {};

        // ---

        template <typename T>
        struct PredConvertible
        {
            template <typename U>
            using Pred = std::bool_constant<Meta::cvref_derived_from_and_convertible_to<U, T>>;
        };
    }

    // Checks if the type `T` contains an element of a type convertible to `Elem` somewhere in it. (`Elem` is usually a reference.)
    // If `T` is a non-reference, it's assumed to be an rvalue reference, as in `std::declval<T>()`.
    // Uses `Meta::cvref_derived_from_and_convertible_to` internally.
    template <typename T, typename Elem>
    concept TypeContainsElemCvref = detail::ContainsType::Check<T, detail::ContainsType::PredConvertible<Elem>::template Pred>::value;

    // Checks if the type `T` contains an element for which `Pred<__>::value` returns true.
    // The predicate should only receive references.
    // If `T` is a non-reference, `&&` is added automatically.
    template <typename T, template <typename> typename Pred>
    concept TypeContainsPred = detail::ContainsType::Check<T, Pred>::value;
}
