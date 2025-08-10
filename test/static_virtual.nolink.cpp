#include "em/refl/static_virtual.h"

struct A
{
    EM_REFL(
        EM_STATIC_VIRTUAL(MyIn, std::derived_from<_em_Derived, _em_Self>)
        (
            (f1, (int) -> float)(return 42;)
            (f2, () -> void)(return;)
        )
        EM_STATIC_VIRTUAL(MyIn2, std::is_base_of_v<_em_Self, _em_Derived>)
        (
            (f1, (int) -> float)(return 42;)
            (f2, () -> void)(return;)
        )
    )
};

struct B : A
{
    EM_REFL()
};
