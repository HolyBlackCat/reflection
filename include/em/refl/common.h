#pragma once

#include "em/meta/cvref.h"
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
