#pragma once

#include "em/meta/type_predicates.h"
#include "em/refl/access/structs.h"
#include "em/refl/contains_type.h"

#include <type_traits>
#include <utility>

// `em::Refl::TypeRecursivelyContainsStaticElemCvref` recursively checks that a type contains some other type.
// This could be implemented in terms of `VisitTypes` from `em/refl/visit_types.h`, but that has worse compilation times.
// Note: This only checks non-static members!

namespace em::Refl
{
    namespace detail::ContainsTypeStatic
    {
        // This uses a cycle: Check -> Check2 -> Check3 -> Check
        // `Check` applies some type transformations (non-reference to rvalue reference) and calls `Check2`.
        // `Check2` checks the filter, and if it returns true, calls `Catch3`. Otherwise returns false.
        // `Check3` loops over all static members.

        template <typename T>
        using PreprocessType = T &&;

        template <typename T, template <typename> typename Pred, template <typename> typename Filter, typename I = std::make_integer_sequence<int, Structs::num_static_members<T>>>
        struct Check3 {};

        template <typename T, template <typename> typename Pred, template <typename> typename Filter>
        struct Check2 : Check3<T, Pred, Filter> {};
        template <typename T, template <typename> typename Pred, template <typename> typename Filter> requires (!Filter<T>::value)
        struct Check2<T, Pred, Filter> : std::false_type {};

        template <typename T, template <typename> typename Pred, template <typename> typename Filter>
        using Check = Check2<PreprocessType<T>, Pred, Filter>;

        // `Check` wrapped in a predicate.
        template <template <typename> typename Pred, template <typename> typename Filter>
        struct CheckPred
        {
            template <typename T>
            using type = Check<T, Pred, Filter>;
        };

        // Recursively apply `Check` to non-static members.
        template <typename T, template <typename> typename Pred, template <typename> typename Filter>
        struct CheckNonStaticMembers : std::bool_constant<TypeRecursivelyContainsPred<T, detail::ContainsTypeStatic::CheckPred<Pred, Filter>, {}, Meta::TraitToPredicate<Filter>>> {};

        template <typename T, template <typename> typename Pred, template <typename> typename Filter>
        struct CheckStaticMemberOrRecurse : CheckNonStaticMembers<T, Pred, Filter> {};
        template <typename T, template <typename> typename Pred, template <typename> typename Filter> requires Pred<T>::value
        struct CheckStaticMemberOrRecurse<T, Pred, Filter> : std::true_type {};

        template <typename T, template <typename> typename Pred, template <typename> typename Filter>
        struct Check3<T, Pred, Filter, std::integer_sequence<int>> : std::false_type {};
        template <typename T, template <typename> typename Pred, template <typename> typename Filter, int I0, int ...I>
        struct Check3<T, Pred, Filter, std::integer_sequence<int, I0, I...>> : Check3<T, Pred, Filter, std::integer_sequence<int, I...>> {};
        template <typename T, template <typename> typename Pred, template <typename> typename Filter, int I0, int ...I> requires CheckStaticMemberOrRecurse<Structs::StaticMemberTypeCvrefMutable<T, I0>, Pred, Filter>::value
        struct Check3<T, Pred, Filter, std::integer_sequence<int, I0, I...>> : std::true_type {};
    }


    // Checks if the type `T` contains a static element for which `Pred<__>::value` returns true.
    // The predicate should only receive references.
    // If `T` is a non-reference, `&&` is added automatically. Since all static variables are reported as lvalue references by default, you probably want to pass an lvalue reference here too.
    // The `Filter` predicate prevents recursion into the types for which it returns false. Also it overrides the `Pred` predicate:
    //   if `Filter` returns false, the `Pred` predicate isn't checked for this type.
    // Note: This only checks static members, but recurses even into non-static members to find more static ones.
    // The only flag respected here is `ignore_root`.
    template <typename T, typename/*TypePredicate*/ Pred, IterationFlags Flags = {}, typename/*TypePredicate*/ Filter = Meta::true_predicate>
    concept TypeRecursivelyContainsStaticPred =
        // Manually check the root element if not disabled.
        (!bool(Flags & IterationFlags::ignore_root) && Filter::template type<T>::value && Pred::template type<T>::value) ||
        // Recurse.
        detail::ContainsTypeStatic::CheckNonStaticMembers<T, Pred::template type, Filter::template type>::value;


    // Checks if the type `T` contains an element of a type convertible to `Elem` somewhere in it. (`Elem` is usually a reference.)
    // If `T` is a non-reference, it's assumed to be an rvalue reference, as in `std::declval<T>()`.
    // Uses `Meta::same_or_derived_from_and_cvref_convertible_to` internally.
    // The `Filter` predicate prevents recursion into the types for which it returns false. Also it overrides the `Pred` predicate:
    //   if `Filter` returns false, the `Pred` predicate isn't checked for this type.
    // Note: This only checks static members, but recurses even into non-static members to find more static ones.
    // The only flag respected here is `ignore_root`.
    template <typename T, typename Elem, IterationFlags Flags = {}, typename/*TypePredicate*/ Filter = Meta::true_predicate>
    concept TypeRecursivelyContainsStaticElemCvref = TypeRecursivelyContainsStaticPred<T, PredTypeMatchesElemCvref<Elem>, Flags, Filter>;


    // This is a predicate version of `TypeRecursivelyContainsStaticPred`. It makes no sense to pass this to `PredTypeMatchesElemCvref`,
    //   because that has its own recursion, but we need this in other places.
    template <Meta::TypePredicate Pred, IterationFlags Flags = {}, Meta::TypePredicate Filter = Meta::true_predicate>
    struct PredTypeRecursivelyContainsStaticPred {template <typename T> using type = std::bool_constant<TypeRecursivelyContainsStaticPred<T, Pred, Flags, Filter>>;};

    // This is a predicate version of `TypeRecursivelyContainsStaticElemCvref`. It makes no sense to pass this to `PredTypeMatchesElemCvref`,
    //   because that has its own recursion, but we need this in other places.
    template <typename Elem, IterationFlags Flags = {}, Meta::TypePredicate Filter = Meta::true_predicate>
    struct PredTypeRecursivelyContainsStaticElemCvref {template <typename T> using type = std::bool_constant<TypeRecursivelyContainsStaticElemCvref<T, Elem, Flags, Filter>>;};
}
