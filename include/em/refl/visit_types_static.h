#pragma once

#include "em/meta/common.h"
#include "em/meta/const_for.h"
#include "em/refl/access/structs.h"

// Provides a way to iterate over the types of members of some type.
// This is similar to `em/refl/contains_type.h`, but provides a bit more control at the cost of longer compilation times.

namespace em::Refl
{
    // Calls `func` on every static subtype of `T`, non-recurisvely. Cvref on `T` are ignored.
    // `func` is `[]<typename T>`.
    // The `T` argument receives each subtype in order (usually a reference, but not always; you usually should assume `&&` for non-references).
    // The return value of `func` is handled according to `LoopBackend`.
    template <typename T, Meta::LoopBackendType LoopBackend, Meta::Deduce..., typename F>
    [[nodiscard]] constexpr decltype(auto) VisitTypesStatic(F &&func)
    {
        // This check isn't really needed, but hopefully it'll make compilation a bit faster?
        if constexpr (Structs::num_static_members<T> > 0)
        {
            return Meta::ConstFor<LoopBackend, Structs::num_static_members<T>>(
                [&]<int I> -> decltype(auto)
                {
                    return func.template operator()<Structs::StaticMemberTypeCvrefMutable<T, I>>();
                }
            );
        }
        else
        {
            return Meta::NoElements<LoopBackend>();
        }
    }
}
