#pragma once

#include "em/macros/utils/forward.h"
#include "em/macros/utils/if_maybe_constexpr.h"
#include "em/macros/utils/lift.h"
#include "em/meta/const_for.h"
#include "em/meta/cvref.h"
#include "em/meta/deduce.h"
#include "em/meta/void.h"
#include "em/refl/classify.h"
#include "em/refl/common.h"

#include <type_traits>

namespace em::Refl
{
    template <typename T, VisitMode Mode = VisitMode::normal, Meta::Deduce..., typename F, typename G = std::nullptr_t>
    [[nodiscard]] constexpr decltype(auto) VisitTypes(F &&func, G &&pop_func = nullptr);

    namespace detail::VisitTypes
    {
        template <typename T, VisitMode Mode = VisitMode::normal, Meta::Deduce...>
        [[nodiscard]] constexpr decltype(auto) VisitTypesLow(auto &&func, auto &&pop_func)
        {
            constexpr Category c = classify_opt<T>;
            if constexpr (c == Category::adjust)
            {
                return (Refl::VisitTypes<Adjust::AdjustedType<T>>)(EM_FWD(func), EM_FWD(pop_func));
            }
            else if constexpr (c == Category::indirect)
            {
                return (Refl::VisitTypes<Indirect::ValueTypeCvref<T>>)(EM_FWD(func), EM_FWD(pop_func));
            }
            else if constexpr (c == Category::structure)
            {
                return Meta::RunEachLambdaEx(
                    // Virtual bases, if `Mode != base_subobject`.
                    [&]<typename TT = T> -> decltype(auto)
                    {
                        if constexpr (Mode != VisitMode::base_subobject)
                        {
                            return Meta::ConstForEachEx(
                                Bases::VirtualBasesFlat<TT>{},
                                [&]<typename Base> -> decltype(auto)
                                {
                                    return (Refl::VisitTypes<Meta::copy_cvref<TT, Base>, VisitMode::base_subobject/*Note!*/>)(EM_FWD(func), EM_FWD(pop_func));
                                }
                            );
                        }
                        else
                        {
                            return Meta::NoElements<false>{};
                        }
                    },
                    // Non-virtual bases.
                    [&]<typename TT = T> -> decltype(auto)
                    {
                        return Meta::ConstForEachEx(
                            Bases::NonVirtualBasesDirect<TT>{},
                            [&]<typename Base> -> decltype(auto)
                            {
                                return (Refl::VisitTypes<Meta::copy_cvref<TT, Base>, VisitMode::base_subobject/*Note!*/>)(EM_FWD(func), EM_FWD(pop_func));
                            }
                        );
                    },
                    // Members, if we have any.
                    [&]<typename TT = T> -> decltype(auto)
                    {
                        if constexpr (!Structs::Type<TT>) // This can happen if the type has bases but no members.
                        {
                            return Meta::NoElements<false>{};
                        }
                        else
                        {
                            return Meta::ConstForEx<Structs::num_members<TT>>(
                                [&]<int I> -> decltype(auto)
                                {
                                    return (Refl::VisitTypes<Structs::MemberTypeCvref<T, I>>)(EM_FWD(func), EM_FWD(pop_func));
                                }
                            );
                        }
                    }
                );
            }
            else if constexpr (c == Category::range)
            {
                return (Refl::VisitTypes<Ranges::ElementTypeCvref<T>>)(EM_FWD(func), EM_FWD(pop_func));
            }
            else if constexpr (c == Category::variant)
            {
                return Meta::ConstForEx<std::variant_size_v<std::remove_cvref_t<T>>>(
                    [&]<std::size_t I> -> decltype(auto)
                    {
                        return (Refl::VisitTypes<Variants::AlternativeTypeCvref<T, I>>)(EM_FWD(func), EM_FWD(pop_func));
                    }
                );
            }
            else if constexpr (c == Category::unknown)
            {
                return Meta::NoElements<false>{};
            }
            else
            {
                static_assert(Meta::always_false<T>, "Unknown category!");
            }
        }
    }

    template <typename T, VisitMode Mode /* = VisitMode::normal */, Meta::Deduce..., typename F, typename G /* = std::nullptr_t */>
    [[nodiscard]] constexpr decltype(auto) VisitTypes(F &&func, G &&pop_func /* = nullptr */)
    {
        return Meta::RunEachLambdaEx(
            EM_EXPR(Meta::InvokeWithVoidPlaceholderResult(EM_EXPR(func.template operator()<T &&>()))),
            [&]<typename TT = T> -> decltype(auto)
            {
                return detail::VisitTypes::VisitTypesLow<T>(EM_FWD(func), EM_FWD(pop_func));
            },
            [&]<typename TT = T> -> decltype(auto)
            {
                if constexpr (!std::is_null_pointer_v<std::remove_cvref_t<G>>)
                    return pop_func.template operator()<TT &&>();
                else
                    return Meta::NoElements<false>{};
            }
        );
    }
}
