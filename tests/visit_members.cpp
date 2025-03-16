#include "em/refl/macros/structs.h"
#include <vector>
#include "em/refl/visit_members.h"

struct A
{
    EM_REFL(
        (std::vector<int>)(a)
        (std::variant<int, float>)(b)
    )
};

struct B : virtual A {EM_REFL()};
struct C {EM_REFL()};
struct D : B, C {EM_REFL()};

[[maybe_unused]] static void foo()
{
    // Just a minimal sanity check.
    em::Refl::VisitMembers<em::Meta::LoopSimple>(D{}, []<em::Refl::VisitDesc Desc, em::Refl::VisitMode Mode>(auto &&){});
}
