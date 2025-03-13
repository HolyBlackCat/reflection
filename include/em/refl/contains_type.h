#pragma once

#include "em/meta/constants.h"
#include "em/meta/cvref.h"
#include "em/meta/type_predicates.h"
#include "em/refl/classify.h"

#include <type_traits>

// `em::Refl::TypeRecursivelyContainsElemCvref` recursively checks that a type contains some other type.
// This could be implemented in terms of `VisitTypes` from `em/refl/visit_types.h`, but that has worse compilation times.

namespace em::Refl
{
    namespace detail::ContainsType
    {
        // This uses a cycle: Check -> Check2 -> Check3 -> Check4 -> Check
        // `Check` applies some type transformations (non-reference to rvalue reference) and calls `Check2`.
        // `Check2` checks the filter, and if it returns true, calls `Catch3`. Otherwise returns false.
        // `Check3` checks the predicate, and if it returns false, calls `Catch4`. Otherwise returns true.
        // `Check4` has category-specific behavior. It runs `Check` for every member of the type. It tries to short-circuit when possible.

        template <typename T, template <typename> typename Pred, template <typename> typename Filter, VisitMode Mode, Category C>
        struct Check4 {};

        template <typename T, template <typename> typename Pred, template <typename> typename Filter, VisitMode Mode>
        struct Check3 : Check4<T, Pred, Filter, Mode, classify_opt<T>> {};
        template <typename T, template <typename> typename Pred, template <typename> typename Filter, VisitMode Mode> requires Pred<T>::value
        struct Check3<T, Pred, Filter, Mode> : std::true_type {};

        template <typename T, template <typename> typename Pred, template <typename> typename Filter, VisitMode Mode>
        struct Check2 : Check3<T, Pred, Filter, Mode> {};
        template <typename T, template <typename> typename Pred, template <typename> typename Filter, VisitMode Mode> requires (!Filter<T>::value)
        struct Check2<T, Pred, Filter, Mode> : std::false_type {};

        template <typename T, template <typename> typename Pred, template <typename> typename Filter, VisitMode Mode = VisitMode::normal>
        using Check = Check2<T &&, Pred, Filter, Mode>;


        // Runs the predicate on all types in the list `T`. Copies the cvref-qualifiers from `Cvref` to each element before checking it.
        template <typename Cvref, typename T, template <typename> typename Pred, template <typename> typename Filter>
        struct CheckBaseTypeListLow {};
        template <typename Cvref, typename T0, typename ...T, template <typename> typename Pred, template <typename> typename Filter>
        struct CheckBaseTypeListLow<Cvref, Meta::TypeList<T0, T...>, Pred, Filter> : CheckBaseTypeListLow<Cvref, Meta::TypeList<T...>, Pred, Filter> {};
        template <typename Cvref, typename T0, typename ...T, template <typename> typename Pred, template <typename> typename Filter> requires Check<Meta::copy_cvref<Cvref, T0>, Pred, Filter, VisitMode::base_subobject/*Note!*/>::value
        struct CheckBaseTypeListLow<Cvref, Meta::TypeList<T0, T...>, Pred, Filter> : std::true_type {};

        // Checks all members of type `T`.
        template <typename T, template <typename> typename Pred, template <typename> typename Filter, typename I = std::make_integer_sequence<int, Structs::num_members<T>>>
        struct CheckMembers {};
        template <typename T, template <typename> typename Pred, template <typename> typename Filter>
        struct CheckMembers<T, Pred, Filter, std::integer_sequence<int>> : std::false_type {};
        template <typename T, template <typename> typename Pred, template <typename> typename Filter, int I0, int ...I>
        struct CheckMembers<T, Pred, Filter, std::integer_sequence<int, I0, I...>> : CheckMembers<T, Pred, Filter, std::integer_sequence<int, I...>> {};
        template <typename T, template <typename> typename Pred, template <typename> typename Filter, int I0, int ...I> requires Check<Structs::MemberTypeCvref<T, I0>, Pred, Filter>::value
        struct CheckMembers<T, Pred, Filter, std::integer_sequence<int, I0, I...>> : std::true_type {};

        // Checks all bases of `T` if `Mode != base_subobject`, otherwise returns false.
        // NOTE! Unlike `VisitTypes`, here we iterate over a flat list of bases, instead of the proper hierarchy.
        // Primarily because the hierarchy is more expensive to compute, and it doesn't matter anyway. On the other hand, `VisitTypes` does respect the hierarchy.
        template <typename T, template <typename> typename Pred, template <typename> typename Filter, VisitMode Mode>
        struct CheckBases : CheckBaseTypeListLow<T, Bases::AllBasesFlat<T>, Pred, Filter> {};
        template <typename T, template <typename> typename Pred, template <typename> typename Filter>
        struct CheckBases<T, Pred, Filter, VisitMode::base_subobject> : std::false_type {};


        // First checks the members of type `T`, then checks it bases.
        template <typename T, template <typename> typename Pred, template <typename> typename Filter, VisitMode Mode>
        struct CheckMembersAndBases : CheckBases<T, Pred, Filter, Mode> {};
        template <typename T, template <typename> typename Pred, template <typename> typename Filter, VisitMode Mode> requires CheckMembers<T, Pred, Filter>::value
        struct CheckMembersAndBases<T, Pred, Filter, Mode> : std::true_type {};

        template <typename T, template <typename> typename Pred, template <typename> typename Filter, typename I = std::make_integer_sequence<int, std::variant_size_v<std::remove_cvref_t<T>>>>
        struct CheckVarAlts {};
        template <typename T, template <typename> typename Pred, template <typename> typename Filter>
        struct CheckVarAlts<T, Pred, Filter, std::integer_sequence<int>> : std::false_type {};
        template <typename T, template <typename> typename Pred, template <typename> typename Filter, int I0, int ...I>
        struct CheckVarAlts<T, Pred, Filter, std::integer_sequence<int, I0, I...>> : CheckVarAlts<T, Pred, Filter, std::integer_sequence<int, I...>> {};
        template <typename T, template <typename> typename Pred, template <typename> typename Filter, int I0, int ...I> requires Check<Variants::AlternativeTypeCvref<T, I0>, Pred, Filter>::value
        struct CheckVarAlts<T, Pred, Filter, std::integer_sequence<int, I0, I...>> : std::true_type {};

        template <typename T, template <typename> typename Pred, template <typename> typename Filter, VisitMode Mode> struct Check4<T, Pred, Filter, Mode, Category::adjust> : Check<Adjust::AdjustedType<T>, Pred, Filter> {};
        template <typename T, template <typename> typename Pred, template <typename> typename Filter, VisitMode Mode> struct Check4<T, Pred, Filter, Mode, Category::indirect> : Check<Indirect::ValueTypeCvref<T>, Pred, Filter> {};
        template <typename T, template <typename> typename Pred, template <typename> typename Filter, VisitMode Mode> struct Check4<T, Pred, Filter, Mode, Category::structure> : CheckMembersAndBases<T, Pred, Filter, Mode> {}; // Only propagate the mode here.
        template <typename T, template <typename> typename Pred, template <typename> typename Filter, VisitMode Mode> struct Check4<T, Pred, Filter, Mode, Category::range> : Check<Ranges::ElementTypeCvref<T>, Pred, Filter> {};
        template <typename T, template <typename> typename Pred, template <typename> typename Filter, VisitMode Mode> struct Check4<T, Pred, Filter, Mode, Category::variant> : CheckVarAlts<T, Pred, Filter> {};
        template <typename T, template <typename> typename Pred, template <typename> typename Filter, VisitMode Mode> struct Check4<T, Pred, Filter, Mode, Category::unknown> : std::false_type {};
    }

    // Checks if the type `T` contains an element for which `Pred<__>::value` returns true.
    // The predicate should only receive references.
    // If `T` is a non-reference, `&&` is added automatically.
    // The `Filter` predicate prevents recursion into the types for which it returns false. Also it overrides the `Pred` predicate:
    //   if `Filter` returns false, the `Pred` predicate isn't checked for this type.
    template <typename T, typename/*TypePredicate*/ Pred, typename/*TypePredicate*/ Filter = Meta::true_predicate>
    concept TypeRecursivelyContainsPred = detail::ContainsType::Check<T, Pred::template type, Filter::template type>::value;


    // `TypeRecursivelyContainsElemCvref` uses this, see below.
    template <typename Elem>
    struct PredTypeMatchesElemCvref
    {
        template <typename Type>
        using type = std::bool_constant<Meta::cvref_derived_from_and_convertible_to<Type, Elem>>;
    };

    // Checks if the type `T` contains an element of a type convertible to `Elem` somewhere in it. (`Elem` is usually a reference.)
    // If `T` is a non-reference, it's assumed to be an rvalue reference, as in `std::declval<T>()`.
    // Uses `Meta::cvref_derived_from_and_convertible_to` internally.
    // The `Filter` predicate prevents recursion into the types for which it returns false. Also it overrides the `Pred` predicate:
    //   if `Filter` returns false, the `Pred` predicate isn't checked for this type.
    template <typename T, typename Elem, typename/*TypePredicate*/ Filter = Meta::true_predicate>
    concept TypeRecursivelyContainsElemCvref = TypeRecursivelyContainsPred<T, PredTypeMatchesElemCvref<Elem>, Filter>;


    // This is a predicate version of `TypeRecursivelyContainsPred`. It makes no sense to pass this to `PredTypeMatchesElemCvref`,
    //   because that has its own recursion, but we need this in other places.
    template <Meta::TypePredicate Pred>
    struct PredTypeRecursivelyContainsPred
    {
        template <typename T>
        using type = std::bool_constant<TypeRecursivelyContainsPred<T, Pred>>;
    };

    // This is a predicate version of `TypeRecursivelyContainsElemCvref`. It makes no sense to pass this to `PredTypeMatchesElemCvref`,
    //   because that has its own recursion, but we need this in other places.
    template <typename Elem>
    struct PredTypeRecursivelyContainsElemCvref
    {
        template <typename T>
        using type = std::bool_constant<TypeRecursivelyContainsElemCvref<T, Elem>>;
    };
}
