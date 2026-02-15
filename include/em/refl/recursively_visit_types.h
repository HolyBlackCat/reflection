#pragma once

#include "em/macros/utils/forward.h"
#include "em/meta/const_for.h"
#include "em/meta/stateful/flag.h"
#include "em/meta/type_predicates.h"
#include "em/refl/common.h"
#include "em/refl/visit_types_static.h"
#include "em/refl/visit_types.h"

namespace em::Refl
{
    // One of the visiting strategies intended mostly for internal use, for the templates below.
    // This simply visits non-static types.
    struct RecursiveTypeVisitorNonStatic
    {
        // Must return `auto` to always instantiate the body for stateful reasons, even though in reality this always ends up returning `void`.
        template <typename T, IterationFlags Flags, Meta::TypePredicate Filter, VisitMode Mode = VisitMode::normal, Meta::Deduce...>
        static constexpr auto Visit(auto &&func)
        {
            if constexpr (Filter::template type<T>::value)
            {
                if constexpr (!bool(Flags & IterationFlags::ignore_root))
                    func.template operator()<T>();

                // Returning `auto` here as well, just in case, to ensure we always instantiate the body.
                (VisitTypes<T, Meta::LoopSimple, Mode>)([&]<typename SubT, VisitDesc Desc> -> auto
                {
                    (Visit<SubT, Flags & ~IterationFlags::ignore_root, Filter, Desc::mode>)(func); // Can't forward `func` in a loop.
                });
            }
        }
    };

    // One of the visiting strategies intended mostly for internal use, for the templates below.
    // This visits static types. And unless `root_is_not_static` is passed, also the non-static ones.
    struct RecursiveTypeVisitorStatic
    {
        // Must return `auto` to always instantiate the body for stateful reasons, even though in reality this always ends up returning `void`.
        template <typename T, IterationFlags Flags, Meta::TypePredicate Filter, VisitMode Mode = VisitMode::normal, Meta::Deduce...>
        static constexpr auto Visit(auto &&func)
        {
            if constexpr (Filter::template type<T>::value)
            {
                if constexpr (!bool(Flags & IterationFlags::root_is_not_static))
                {
                    // Run the callback.
                    if constexpr (!bool(Flags & IterationFlags::ignore_root))
                        func.template operator()<T>();

                    // Recurse into non-static types.
                    // Returning `auto` here as well, just in case, to ensure we always instantiate the body.
                    (VisitTypes<T, Meta::LoopSimple, Mode>)([&]<typename SubT, VisitDesc Desc> -> auto
                    {
                        (Visit<SubT, Flags & ~IterationFlags::ignore_root, Filter, Desc::mode>)(func); // Can't forward `func` in a loop.
                    });
                }

                // Recurse into static types.
                // Returning `auto` here as well, just in case, to ensure we always instantiate the body.
                // Note, `VisitStaticTypes()` doesn't take a `VisitMode` template parameter, unlike `VisitTypes()`. And also doesn't report a mode to the lambda,
                //   so we don't pass any to the underlying recursive `Visit()` call.
                (VisitStaticTypes<T, Meta::LoopSimple>)([&]<typename SubT> -> auto
                {
                    (Visit<SubT, Flags & ~IterationFlags::ignore_root & ~IterationFlags::root_is_not_static, Filter>)(func); // Can't forward `func` in a loop.
                });
            }
        }
    };

    namespace detail::RecursivelyVisitTypes
    {
        template <typename T>
        using AdjustType = T &&;

        // A tag for stateful flags.
        template <typename T, typename Strategy, Meta::TypePredicate Pred, IterationFlags Flags, Meta::TypePredicate Filter, VisitMode Mode>
        struct ContainsTypeTag {};

        template <typename T, typename Strategy, Meta::TypePredicate Pred, IterationFlags Flags, Meta::TypePredicate Filter, VisitMode Mode>
        struct ContainsTypeFunc
        {
            // Returning `auto` to always instantiate the body.
            template <typename Elem>
            constexpr auto operator()()
            {
                if constexpr (Pred::template type<Elem>::value)
                    (void)Meta::Stateful::Flag::Set<ContainsTypeTag<T, Strategy, Pred, Flags, Filter, Mode>>{};
            }
        };
    }

    // See `TypeRecursivelyContains[Static]Pred` below. This is a generalized version that accepts a visiting strategy.
    template <typename T, typename Strategy, typename/*TypePredicate*/ Pred, IterationFlags Flags = {}, typename/*TypePredicate*/ Filter = Meta::true_predicate, VisitMode Mode = VisitMode::normal>
    concept TypeRecursivelyContainsPredWithStrategy = (
        decltype(
            Strategy::template Visit<detail::RecursivelyVisitTypes::AdjustType<T>, Flags, Filter, Mode>(
                detail::RecursivelyVisitTypes::ContainsTypeFunc<detail::RecursivelyVisitTypes::AdjustType<T>, Strategy, Pred, Flags, Filter, Mode>{}
            )
        )(),
        Meta::Stateful::Flag::value<detail::RecursivelyVisitTypes::ContainsTypeTag<detail::RecursivelyVisitTypes::AdjustType<T>, Strategy, Pred, Flags, Filter, Mode>>
    );

    // Returns true if `T` recrusively contains at least one element type matching `Pred`.
    // By default `T` itself also counts, unless you specify `Flags::ignore_root`.
    // `Filter` rejects whole tree branches. If it's false, the `Pred` is not checked.
    // `&&` on T` is implied.
    template <typename T, typename/*TypePredicate*/ Pred, IterationFlags Flags = {}, typename/*TypePredicate*/ Filter = Meta::true_predicate, VisitMode Mode = VisitMode::normal>
    concept TypeRecursivelyContainsPred = TypeRecursivelyContainsPredWithStrategy<T, RecursiveTypeVisitorNonStatic, Pred, Flags, Filter, Mode>;

    // Same, but for static types.
    template <typename T, typename/*TypePredicate*/ Pred, IterationFlags Flags = {}, typename/*TypePredicate*/ Filter = Meta::true_predicate, VisitMode Mode = VisitMode::normal>
    concept TypeRecursivelyContainsStaticPred = TypeRecursivelyContainsPredWithStrategy<T, RecursiveTypeVisitorStatic, Pred, Flags, Filter, Mode>;


    // `TypeRecursivelyContainsElemCvref` uses this, see below.
    template <typename Elem>
    struct PredTypeMatchesElemCvref
    {
        template <typename Type>
        using type = std::bool_constant<Meta::same_or_derived_from_and_cvref_convertible_to<Type, Elem>>;
    };


    // Returns true if `T` recrusively contains at least one element type same as `Elem` (which should be a reference) or with compatible cvref.
    // By default `T` itself also counts, unless you specify `Flags::ignore_root`.
    // `Filter` rejects whole tree branches. If it's false, the `Pred` is not checked.
    // `&&` on T` is implied.
    template <typename T, typename Elem, IterationFlags Flags = {}, typename/*TypePredicate*/ Filter = Meta::true_predicate, VisitMode Mode = VisitMode::normal>
    concept TypeRecursivelyContainsElemCvref = TypeRecursivelyContainsPred<T, PredTypeMatchesElemCvref<Elem>, Flags, Filter, Mode>;

    // Same, but for static types.
    template <typename T, typename Elem, IterationFlags Flags = {}, typename/*TypePredicate*/ Filter = Meta::true_predicate, VisitMode Mode = VisitMode::normal>
    concept TypeRecursivelyContainsStaticElemCvref = TypeRecursivelyContainsStaticPred<T, PredTypeMatchesElemCvref<Elem>, Flags, Filter, Mode>;


    // Convenience recursive predicates.

    template <Meta::TypePredicate Pred, IterationFlags Flags = {}, Meta::TypePredicate Filter = Meta::true_predicate, VisitMode Mode = VisitMode::normal>
    struct PredTypeRecursivelyContainsPred
    {
        template <typename T>
        using type = std::bool_constant<TypeRecursivelyContainsPred<T, Pred, Flags, Filter, Mode>>;
    };

    template <Meta::TypePredicate Pred, IterationFlags Flags = {}, Meta::TypePredicate Filter = Meta::true_predicate, VisitMode Mode = VisitMode::normal>
    struct PredTypeRecursivelyContainsStaticPred
    {
        template <typename T>
        using type = std::bool_constant<TypeRecursivelyContainsStaticPred<T, Pred, Flags, Filter, Mode>>;
    };

    template <typename Elem, IterationFlags Flags = {}, Meta::TypePredicate Filter = Meta::true_predicate, VisitMode Mode = VisitMode::normal>
    struct PredTypeRecursivelyContainsElemCvref
    {
        template <typename T>
        using type = std::bool_constant<TypeRecursivelyContainsElemCvref<T, Elem, Flags, Filter, Mode>>;
    };

    template <typename Elem, IterationFlags Flags = {}, Meta::TypePredicate Filter = Meta::true_predicate, VisitMode Mode = VisitMode::normal>
    struct PredTypeRecursivelyContainsStaticElemCvref
    {
        template <typename T>
        using type = std::bool_constant<TypeRecursivelyContainsStaticElemCvref<T, Elem, Flags, Filter, Mode>>;
    };


    // Collecting a list of matching types.

    namespace detail::RecursivelyVisitTypes
    {
        // A tag for stateful lists.
        template <typename T, typename Strategy, template <typename> typename Map, Meta::TypePredicate Pred, IterationFlags Flags, Meta::TypePredicate Filter, VisitMode Mode>
        struct TypeListTag {};

        template <typename T, typename Strategy, template <typename> typename Map, Meta::TypePredicate Pred, IterationFlags Flags, Meta::TypePredicate Filter, VisitMode Mode>
        struct TypeListFunc
        {
            // Returning `auto` to always instantiate the body.
            template <typename Elem>
            constexpr auto operator()()
            {
                if constexpr (Pred::template type<Map<Elem>>::value)
                    (void)Meta::Stateful::List::PushBack<TypeListTag<T, Strategy, Map, Pred, Flags, Filter, Mode>, Map<Elem>>{};
            }
        };

        template <typename T, typename Strategy, template <typename> typename Map, Meta::TypePredicate Pred, IterationFlags Flags, Meta::TypePredicate Filter, VisitMode Mode>
        using FillTypeList = decltype(
            Strategy::template Visit<AdjustType<T>, Flags, Filter, Mode>(
                TypeListFunc<AdjustType<T>, Strategy, Map, Pred, Flags, Filter, Mode>{}
            )
        );
    }

    // This is a generalized version of `RecursivelyNested[Static]Types` that takes a custom `Strategy`, see those below.
    template <typename T, typename Strategy, template <typename> typename Map, Meta::TypePredicate Pred, IterationFlags Flags = {}, Meta::TypePredicate Filter = Meta::true_predicate, VisitMode Mode = VisitMode::normal>
    using RecursivelyNestedTypesWithStrategy = decltype(detail::RecursivelyVisitTypes::FillTypeList<T, Strategy, Map, Pred, Flags, Filter, Mode>{}, Meta::Stateful::List::Elems<detail::RecursivelyVisitTypes::TypeListTag<detail::RecursivelyVisitTypes::AdjustType<T>, Strategy, Map, Pred, Flags, Filter, Mode>>{});

    // Returns true if `T` recrusively contains at least one element type same as `Elem` (which should be a reference) or with compatible cvref.
    // By default `T` itself also counts, unless you specify `Flags::ignore_root`.
    // `Filter` rejects whole tree branches. If it's false, the `Pred` is not checked.
    // `&&` on T` is implied.
    template <typename T, template <typename> typename Map, Meta::TypePredicate Pred, IterationFlags Flags = {}, Meta::TypePredicate Filter = Meta::true_predicate, VisitMode Mode = VisitMode::normal>
    using RecursivelyNestedTypes = RecursivelyNestedTypesWithStrategy<T, RecursiveTypeVisitorNonStatic, Map, Pred, Flags, Filter, Mode>;

    // Same, but for static types.
    template <typename T, template <typename> typename Map, Meta::TypePredicate Pred, IterationFlags Flags = {}, Meta::TypePredicate Filter = Meta::true_predicate, VisitMode Mode = VisitMode::normal>
    using RecursivelyNestedStaticTypes = RecursivelyNestedTypesWithStrategy<T, RecursiveTypeVisitorStatic, Map, Pred, Flags, Filter, Mode>;


    // Visiting the list of matching types.

    // See `RecursivelyVisit[Static]TypesMatchingPred()` below. This is a generalized version that takes a `Strategy` parameter.
    template <typename T, typename Strategy, template <typename> typename Map, Meta::TypePredicate Pred, IterationFlags Flags = {}, Meta::LoopBackendType LoopBackend = Meta::LoopSimple, Meta::TypePredicate Filter = Meta::true_predicate, VisitMode Mode = VisitMode::normal, Meta::Deduce...>
    constexpr decltype(auto) RecursivelyVisitTypesMatchingPredWithStrategy(auto &&func)
    {
        return Meta::ConstForEach<LoopBackend>(RecursivelyNestedTypesWithStrategy<T, Strategy, Map, Pred, Flags, Filter, Mode>{}, EM_FWD(func));
    }

    // Visit non-static types.
    // `func` is `[]<typename ElemT> -> ...`.
    template <typename T, template <typename> typename Map, Meta::TypePredicate Pred, Meta::LoopBackendType LoopBackend = Meta::LoopSimple, IterationFlags Flags = {}, Meta::TypePredicate Filter = Meta::true_predicate, VisitMode Mode = VisitMode::normal, Meta::Deduce...>
    constexpr decltype(auto) RecursivelyVisitTypesMatchingPred(auto &&func)
    {
        return (RecursivelyVisitTypesMatchingPredWithStrategy<T, RecursiveTypeVisitorNonStatic, Map, Pred, Flags, LoopBackend, Filter, Mode>)(EM_FWD(func));
    }

    // Same but for static types.
    template <typename T, template <typename> typename Map, Meta::TypePredicate Pred, Meta::LoopBackendType LoopBackend = Meta::LoopSimple, IterationFlags Flags = {}, Meta::TypePredicate Filter = Meta::true_predicate, VisitMode Mode = VisitMode::normal, Meta::Deduce...>
    constexpr decltype(auto) RecursivelyVisitStaticTypesMatchingPred(auto &&func)
    {
        return (RecursivelyVisitTypesMatchingPredWithStrategy<T, RecursiveTypeVisitorStatic, Map, Pred, Flags, LoopBackend, Filter, Mode>)(EM_FWD(func));
    }
}
