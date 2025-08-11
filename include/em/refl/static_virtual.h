#pragma once

#include "em/macros/meta/common.h"
#include "em/macros/portable/warnings.h"
#include "em/meta/type_name.h"
#include "em/refl/macros/structs.h"

#include <fmt/format.h>

#include <functional>
#include <map>
#include <stdexcept> // IWYU pragma: keep, clearly used below.
#include <string>


// Provides the `EM_STATIC_VIRTUAL()` macro, see below.

namespace em::Refl::StaticVirtual
{
    // Whether `I` is a `Base::Interface` created by `EM_REGISTER_DERIVED(Interface, ...)` in the class `Bsae`.
    template <typename I>
    concept Interface = requires{typename I::_em_IsStaticVirtuallInterface;};

    // `I` is `Base::Interface`, where `Interface` is the identifier that was passed to `EM_REGISTER_DERIVED()` as the interface name.
    // Stores a list of implementations of this interface, for the classes derived from `Base`.
    // The keys are the derived class names from `em::TypeName()`.
    template <Interface I>
    using Map = std::map<std::string, const I *, std::less<>>;

    namespace detail
    {
        template <Interface I>
        [[nodiscard]] Map<I> &GetDerivedMap()
        {
            static Map<I> ret;
            return ret;
        }

        template <Interface I, typename D, typename DI>
        void RegisterDerived()
        {
            static const DI impl{};
            if (!GetDerivedMap<I>().try_emplace(std::string(em::Meta::TypeName<D>()), &impl).second)
                throw std::runtime_error(fmt::format("Internal error: Duplicate derived class registered: {}", em::Meta::TypeName<D>()));
        }
    }

    // Returns the implementations of the interface `I` for all the derived classes matching its condition.
    template <Interface I>
    [[nodiscard]] const Map<I> &GetMap()
    {
        return detail::GetDerivedMap<I>();
    }
}

// Type-erases arbitrary information about every class derived from this that has `EM_REFL()` in it (including this class itself),
//   if it satisfies the condition you specified.
// Use `em::Refl::StaticVirtual::GetMap()` to then get the list of those classes and the interface implementations for them.
//
// Usage, inside of `EM_REFL(...)` of the base class:
//     EM_STATIC_VIRTUAL(InterfaceName, cond...)
//     (
//         func1
//         func2
//         ...
//     )
//
// Where:
//   `InterfaceName` is a class name for an interface that will be created at this location.
//   `cond...` is a condition, without parentheses, in terms of `_em_Self` and `_em_Derived`, that checks the inheritance.
//     Typically this would be either `std::derived_from<_em_Derived, _em_Self>` or `std::is_base_of<_em_Self, _em_Derived>`. Not checking either of those
//       will cause weird behavior, where stray non-derived classes will appear in the lists.
//     Here `_em_Self` is always the same as the enclosing class, and you can use that class name explicitly if you prefer.
//     You can add some additional checks here, like rejecting abstract classes.
//     You might also want to reject `std::is_same_v<_em_Self, _em_Derived>`, since otherwise this class itself will be registered too.
//
// And where each `func` is:
//     (name, (params...) -> ret)
//     (
//         body...
//     )
// Where:
//   `name` is the function name.
//   `(params...)` is a function parameter list, the usual `(type1 name1, type2 name2, ...)`.
//   `ret` is the return type.
//   `body...` is the function body. Can contain multiple statements. This is templated over `_em_Derived`, do something with it.
//
// Note that this is currently designed to be placed in `EM_REFL()` for no real reason, other than guaranteeing that you can use `_em_Self` in the condition,
//   which is provided by the reflection.
#define EM_STATIC_VIRTUAL(interface_name_, .../*cond*/) DETAIL_EM_STATIC_VIRTUAL_3 DETAIL_EM_STATIC_VIRTUAL_1(interface_name_, __VA_ARGS__)
#define DETAIL_EM_STATIC_VIRTUAL_1(interface_name_, .../*cond*/) (, interface_name_, (__VA_ARGS__ DETAIL_EM_STATIC_VIRTUAL_2
#define DETAIL_EM_STATIC_VIRTUAL_2(seq_) ), seq_)

#define DETAIL_EM_STATIC_VIRTUAL_3(n, interface_name_, cond_, seq_) \
    EM_REFL_VERBATIM_LOW(body, em_static_virtual,, \
        struct interface_name_ \
        { \
            using _em_IsStaticVirtuallInterface = void; /* Mark the class so we know it's a valid interface. */\
            virtual ~interface_name_() = default; \
            EM_END(DETAIL_EM_REGISTER_DERIVED_BODY_INTERFACE_A seq_) \
        }; \
        template <typename _em_Derived> \
        struct EM_CAT(_em_RegisterDerivedImpl, interface_name_) : interface_name_ \
        { \
            EM_END(DETAIL_EM_REGISTER_DERIVED_BODY_IMPL_A seq_) \
        }; \
        EM_REFL_INHERITANCE_HOOK(EM_CAT(em_register_derived_, interface_name_), cond_, ::em::Refl::StaticVirtual::detail::RegisterDerived<interface_name_, _em_Derived, EM_CAT(_em_RegisterDerivedImpl, interface_name_)<_em_Derived>>();)\
    )

#define DETAIL_EM_REGISTER_DERIVED_BODY_INTERFACE_A(name_, ...) EM_SILENCE_UNUSED_ATTRIBUTE( [[nodiscard]] ) virtual auto name_ DETAIL_EM_REGISTER_DERIVED_INJECT_CONST __VA_ARGS__ = 0; DETAIL_EM_REGISTER_DERIVED_BODY_INTERFACE_B
#define DETAIL_EM_REGISTER_DERIVED_BODY_INTERFACE_B(...) DETAIL_EM_REGISTER_DERIVED_BODY_INTERFACE_A
#define DETAIL_EM_REGISTER_DERIVED_BODY_INTERFACE_A_END
#define DETAIL_EM_REGISTER_DERIVED_BODY_INTERFACE_B_END

#define DETAIL_EM_REGISTER_DERIVED_BODY_IMPL_A(name_, ...) EM_SILENCE_UNUSED_ATTRIBUTE( [[nodiscard]] ) auto name_ DETAIL_EM_REGISTER_DERIVED_INJECT_CONST __VA_ARGS__ override DETAIL_EM_REGISTER_DERIVED_BODY_IMPL_B
#define DETAIL_EM_REGISTER_DERIVED_BODY_IMPL_B(...) {__VA_ARGS__} DETAIL_EM_REGISTER_DERIVED_BODY_IMPL_A
#define DETAIL_EM_REGISTER_DERIVED_BODY_IMPL_A_END
#define DETAIL_EM_REGISTER_DERIVED_BODY_IMPL_B_END

#define DETAIL_EM_REGISTER_DERIVED_INJECT_CONST(...) (__VA_ARGS__) const
