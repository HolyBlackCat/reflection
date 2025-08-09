#pragma once

#include "em/refl/macros/structs/reg_derived.h"

namespace em::Refl::Derived
{
    // `I` is `Base::Interface`, where `Interface` is the identifier that was passed to `EM_REGISTER_DERIVED()` as the interface name.
    // Returns a map of implementations of this interface, where keys are the derived class names from `em::TypeName()`.
    template <Interface I>
    [[nodiscard]] const DerivedMap<I> &GetMap()
    {
        return detail::GetDerivedMap<I>();
    }
}
