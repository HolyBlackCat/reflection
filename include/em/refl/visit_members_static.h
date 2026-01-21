#pragma once

#include "em/meta/common.h"
#include "em/meta/const_for.h"
#include "em/refl/access/structs.h"

namespace em::Refl
{
    // Calls `func` on every static member of `T`, non-recurisvely.
    // `func` is `[](auto &&member)` (and you can a template parameter for the type of `member`).
    // The return value of `func` is handled according to `LoopBackend`.
    template <typename T, Meta::LoopBackendType LoopBackend, Meta::Deduce..., typename F>
    [[nodiscard]] constexpr decltype(auto) VisitStaticMembers(F &&func)
    {
        // This check isn't really needed, but hopefully it'll make compilation a bit faster?
        if constexpr (Structs::num_static_members<T> > 0)
        {
            return Meta::ConstFor<LoopBackend, Structs::num_static_members<T>>(
                [&]<int I> -> decltype(auto)
                {
                    return func(Structs::GetStaticMemberMutable<T, I>());
                }
            );
        }
        else
        {
            return Meta::NoElements<LoopBackend>();
        }
    }
}
