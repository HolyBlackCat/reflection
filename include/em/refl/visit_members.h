#pragma once

#include "em/macros/utils/forward.h"
#include "em/meta/const_for.h"
#include "em/meta/deduce.h"
#include "em/refl/classify.h"
#include "em/refl/common.h"

#include <type_traits>

namespace em::Refl
{
    // Calls `func` on every member of `T`, non-recurisvely.
    // `func` is `[]<VisitDesc Desc>(auto &&member)` (or you can add another template parameter for the `member` type).
    // `Desc` receives one of the `Visiting...` tags describing what this member is (defined in `em/refl/common.h`). For most type categories this is `VisitingOther`.
    // The return value of `func` is handled according to `LoopBackend`.
    // NOTE: When visitng recursively, must pass `Desc::mode` as the mode to any recursive calls, instead of the default mode.
    template <Meta::LoopBackendType LoopBackend, IterationFlags Flags = {}, VisitMode Mode = VisitMode::normal, Meta::Deduce..., typename T, typename F>
    requires (!LoopBackend::is_reverse) || (bool(Flags & IterationFlags::fallback_to_not_reverse)) || Ranges::BackwardIterableOrNonRange<T>
    [[nodiscard]] constexpr decltype(auto) VisitMembers(T &&object, F &&func)
    {
        constexpr Category c = classify_opt<T>;

        if constexpr (c == Category::adjust)
        {
            return EM_FWD(func).template operator()<VisitingOther>(Adjust::Adjust(EM_FWD(object)));
        }
        else if constexpr (c == Category::indirect)
        {
            if constexpr (Indirect::AlwaysHasValue<T>)
            {
                return EM_FWD(func).template operator()<VisitingOther>(Indirect::GetValue(EM_FWD(object)));
            }
            else
            {
                if (Indirect::HasValue(object)) // `HasValue` takes the object by const reference, so we don't need to forward it.
                    return EM_FWD(func).template operator()<VisitingOther>(Indirect::GetValue(EM_FWD(object)));
                else
                    return Meta::NoElements<LoopBackend>();
            }
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
                                return func.template operator()<VisitingVirtualBase>(Bases::CastToBase<Base>(object));
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
                            return func.template operator()<VisitingDirectNonVirtualBase>(Bases::CastToBase<Base>(object));
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
                                return func.template operator()<VisitingClassMember<I>>(Structs::GetMemberMutable<I>(object));
                            }
                        );
                    }
                }
            );
        }
        else if constexpr (c == Category::range)
        {
            return Ranges::ForEach<LoopBackend, Flags>(EM_FWD(object), [&](auto &&elem) -> decltype(auto) {return func.template operator()<VisitingOther>(EM_FWD(elem));});
        }
        else if constexpr (c == Category::variant)
        {
            return Meta::ConstFor<LoopBackend, std::variant_size_v<std::remove_cvref_t<T>>>(
                [&]<std::size_t I> -> decltype(auto)
                {
                    // Not forwarding the `func` in a loop.
                    return func.template operator()<VisitingVariantAlternative<I>>(Variants::Get<I>(EM_FWD(object)));
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
