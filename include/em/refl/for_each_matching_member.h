#pragma once

#include "em/meta/const_for.h"
#include "em/meta/deduce.h"
#include "em/refl/common.h"
#include "em/refl/contains_type.h"
#include "em/refl/visit_members.h"

namespace em::Refl
{
    template <typename Elem, Meta::LoopBackendType LoopBackend = Meta::LoopSimple, VisitMode Mode = VisitMode::normal, Meta::Deduce..., typename T, typename F>
    constexpr decltype(auto) ForEachMatchingElem(T &&input, F &&func)
    {
        if (Meta::cvref_derived_from_and_convertible_to<T &&, Elem &&>)
        {
            func(EM_FWD(input));
        }
        else if constexpr (TypeContainsElemCvref<T &&, Elem &&>)
        {
            (VisitMembers<LoopBackend, Mode>)(EM_FWD(input), [&]<typename Desc, VisitMode Submode>(auto &&member) -> decltype(auto)
            {
                return (ForEachMatchingElem<Elem, LoopBackend, Submode>)(EM_FWD(member), EM_FWD(func));
            });
        }
    }
}
