#pragma once

#include "em/macros/utils/forward.h"
#include "em/meta/const_for.h"
#include "em/meta/cvref.h"
#include "em/meta/deduce.h"
#include "em/refl/classify.h"
#include "em/refl/common.h"

#include <type_traits>

// Provides a way to iterate over the types of members of some type.
// This is similar to `em/refl/contains_type.h`, but provides a bit more control at the cost of longer compilation times.

namespace em::Refl
{
    // Calls `func` on every subtype of `T`, non-recurisvely. If `T` is a non-reference, adds `&&` automatically.
    // `func` is `[]<typename T, VisitDesc Desc, VisitMode Mode>`.
    // The `T` argument receives each subtype in order (usually a reference, but not always; you usually should assume `&&` for non-references).
    // `Desc` receives one of the `Visiting...` tags describing what this member is (defined in `em/refl/common.h`). For most type categories this is `VisitingOther`.
    // `Mode` receives the mode that you should pass to any recursive calls to `VisitTypes(...)`. Nested calls should not use the default mode.
    // The return value of `func` is handled according to `LoopBackend`.
    template <typename T, Meta::LoopBackendType LoopBackend, VisitMode Mode = VisitMode::normal, Meta::Deduce..., typename F>
    [[nodiscard]] constexpr decltype(auto) VisitTypes(F &&func)
    {
        // Note that most uses of `T` here should be as `T &&`.

        constexpr Category c = classify_opt<T>;

        if constexpr (c == Category::adjust)
        {
            return EM_FWD(func).template operator()<Adjust::AdjustedType<T &&>, VisitingOther, VisitMode::normal>();
        }
        else if constexpr (c == Category::indirect)
        {
            return EM_FWD(func).template operator()<Indirect::ValueTypeCvref<T &&>, VisitingOther, VisitMode::normal>();
        }
        else if constexpr (c == Category::structure)
        {
            return Meta::RunEachFunc<LoopBackend>(
                // Virtual bases, if `Mode != base_subobject`.
                [&]<typename TT = T> -> decltype(auto)
                {
                    if constexpr (Mode != VisitMode::base_subobject)
                    {
                        return Meta::ConstForEach<LoopBackend>(
                            Bases::VirtualBasesFlat<TT>{},
                            [&]<typename Base> -> decltype(auto)
                            {
                                // Not forwarding the `func` in a loop.
                                return func.template operator()<Meta::copy_cvref<TT &&, Base>, VisitingVirtualBase, VisitMode::base_subobject>();
                            }
                        );
                    }
                    else
                    {
                        return Meta::NoElements<LoopBackend>();
                    }
                },
                // Non-virtual bases.
                [&]<typename TT = T> -> decltype(auto)
                {
                    return Meta::ConstForEach<LoopBackend>(
                        Bases::NonVirtualBasesDirect<TT>{},
                        [&]<typename Base> -> decltype(auto)
                        {
                            // Not forwarding the `func` in a loop.
                            return func.template operator()<Meta::copy_cvref<TT &&, Base>, VisitingDirectNonVirtualBase, VisitMode::base_subobject>();
                        }
                    );
                },
                // Members, if we have any.
                [&]<typename TT = T> -> decltype(auto)
                {
                    if constexpr (!Structs::Type<TT>) // This can happen if the type has bases but no members.
                    {
                        return Meta::NoElements<LoopBackend>();
                    }
                    else
                    {
                        return Meta::ConstFor<LoopBackend, Structs::num_members<TT>>(
                            [&]<int I> -> decltype(auto)
                            {
                                // Not forwarding the `func` in a loop.
                                return func.template operator()<Structs::MemberTypeCvref<TT &&, I>, VisitingClassMember<I>, VisitMode::normal>();
                            }
                        );
                    }
                }
            );
        }
        else if constexpr (c == Category::range)
        {
            return EM_FWD(func).template operator()<Ranges::ElementTypeCvref<T>, VisitingOther, VisitMode::normal>();
        }
        else if constexpr (c == Category::variant)
        {
            return Meta::ConstFor<LoopBackend, std::variant_size_v<std::remove_cvref_t<T>>>(
                [&]<std::size_t I> -> decltype(auto)
                {
                    // Not forwarding the `func` in a loop.
                    return func.template operator()<Variants::AlternativeTypeCvref<T, I>, VisitingVariantAlternative<I>, VisitMode::normal>();
                }
            );
        }
        else if constexpr (c == Category::unknown)
        {
            return Meta::NoElements<LoopBackend>();
        }
        else
        {
            static_assert(Meta::always_false<T>, "Unknown category!");
        }
    }
}
