#pragma once

#include "em/macros/utils/flag_enum.h"
#include "em/meta/common.h"
#include "em/meta/lists.h"

#include <concepts>

namespace em::Refl
{
    // Inherit your attribute types from this.
    struct BasicAttribute {};

    // Valid attribute types (cv-unqualified and inherited from `BasicAttribute`),
    // or base classes for attributes (those normally have protected destructors).
    template <typename T>
    concept AttributeOrBase = std::derived_from<T, BasicAttribute> && Meta::cv_unqualified<T> && !std::is_same_v<T, BasicAttribute>;

    // Valid attribute types.
    template <typename T>
    concept Attribute = AttributeOrBase<T> && std::default_initializable<T>;

    // Describes a struct member type and a list of attributes associated with it.
    // When customizing this, pass `Type = void` to avoid overriding the type (to use `decltype(...)` on the getter as by default).
    // You don't have to use this for customization, you can just copy the member names.
    template <typename Type, Attribute ...Attrs>
    struct MemberInfo
    {
        using type = Type; // Optional, if not specified we guess the type from the getter.
        using attrs = Meta::TypeList<Attrs...>; // Optional, if not specified we assume no attributes. You can use any variadic template for the type list.
    };


    // --- Visiting:

    enum class VisitMode
    {
        normal,
        // A class sets this when visiting its own bases. This means either "don't visit virtual bases",
        //   or "don't visit any bases" (if the bases are visited as a flat list, e.g. if we're just looking for existence of some type).
        base_subobject,
    };

    // Iteration direction for loops.
    enum class IterationFlags
    {
        // If we try to iterate over a range backwards, and that range type doesn't support that, fall back to the forward iteration.
        fallback_to_not_reverse = 1 << 0,

        // Recursive functions use those: [

        // Don't call the lambda on the top-level object even if it matches the predicate.
        ignore_root = 1 << 1,

        // The specified predicate automatically handles the bases.
        // This means that if the predicate returned true on the derived class, we shouldn't check it and call the lambda again for the bases.
        predicate_finds_bases = 1 << 2,

        // ]
    };
    EM_FLAG_ENUM(IterationFlags)

    // Element descriptions:

    // This is the common base class for the tags.
    struct BasicVisitingTag
    {
        // The recommended mode for visiting recursively from here.
        static constexpr VisitMode mode = VisitMode::normal;

      protected:
        BasicVisitingTag() = default;
    };

    // Matches any visiting tag.
    // Note the use of `std::default_initializable` to reject bases with private destructors.
    template <typename T>
    concept VisitDesc = std::derived_from<T, BasicVisitingTag> && std::default_initializable<T>;

    // For classes:
    struct VisitingAnyBase : BasicVisitingTag
    {
        static constexpr VisitMode mode = VisitMode::base_subobject;

      protected:
        VisitingAnyBase() = default;
    };

    struct VisitingDirectNonVirtualBase : VisitingAnyBase {VisitingDirectNonVirtualBase() = default;};
    struct VisitingVirtualBase : VisitingAnyBase {VisitingVirtualBase() = default;};

    struct VisitingSomeClassMember : BasicVisitingTag {protected: VisitingSomeClassMember() = default;};
    // Here `::type = Class` is the enclosing class, and `::value = I` is the member index.
    // We pass them here to simplify querying the attributes on the members.
    template <int I, Meta::cvref_unqualified Class> struct VisitingClassMember : VisitingSomeClassMember, std::integral_constant<int, I> {VisitingClassMember() = default; using type = Class;};
    // For variants:
    struct VisitingSomeVariantAlternative : BasicVisitingTag {protected: VisitingSomeVariantAlternative() = default;};
    template <int I> struct VisitingVariantAlternative : VisitingSomeVariantAlternative, std::integral_constant<int, I> {VisitingVariantAlternative() = default;};
    // Other:
    struct VisitingOther : BasicVisitingTag {VisitingOther() = default;};


    namespace detail
    {
        // Use this with `em::Meta::DetectBases`.
        struct StructBasesTag {};
    }

    namespace custom
    {
        // This is a helper type convertible to `int`.
        // All of our ADL customization points (functions) have `int` as the first parameter, to which we pass this type.
        // This lets you implement those functions in this namespace, in addition to the normal ADL namespaces.
        struct AdlDummy
        {
            constexpr operator int() {return 0;}
        };
    }
}
