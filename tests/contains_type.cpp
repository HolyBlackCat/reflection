#include "em/refl/contains_type.h"
#include "em/refl/macros/structs.h"
#include "em/refl/visit_types.h"

#include <memory>
#include <vector>

namespace
{
    template <typename T, em::Refl::VisitMode = em::Refl::VisitMode::normal>
    constexpr bool contains_type_using_visit(const em::Meta::CvrefFlagsAndType &desc)
    {
        if (em::Meta::SameTypeAndDescIsConstructibleFrom<T>(desc))
            return true;

        return (bool)em::Refl::VisitTypes<T, em::Meta::LoopAnyOf<>>(
            [&]<typename U, typename Desc, em::Refl::VisitMode Mode>()
            {
                return contains_type_using_visit<U, Mode>(desc);
            }
        );
    }


    template <typename T, typename Elem>
    constexpr bool contains_type = []{
        constexpr bool a = em::Refl::TypeContainsElemCvref<T, Elem>;
        constexpr bool b = contains_type_using_visit<T>(em::Meta::type_to_desc<Elem>);
        static_assert(a == b);
        return b;
    }();
}

// Same type with different cvref-qualifiers.
static_assert(contains_type<      int   ,       int   > == true);
static_assert(contains_type<      int & ,       int   > == true);
static_assert(contains_type<      int &&,       int   > == true);
static_assert(contains_type<const int   ,       int   > == true);
static_assert(contains_type<const int & ,       int   > == true);
static_assert(contains_type<const int &&,       int   > == true);
static_assert(contains_type<      int   ,       int & > == false);
static_assert(contains_type<      int & ,       int & > == true);
static_assert(contains_type<      int &&,       int & > == false);
static_assert(contains_type<const int   ,       int & > == false);
static_assert(contains_type<const int & ,       int & > == false);
static_assert(contains_type<const int &&,       int & > == false);
static_assert(contains_type<      int   ,       int &&> == true);
static_assert(contains_type<      int & ,       int &&> == false);
static_assert(contains_type<      int &&,       int &&> == true);
static_assert(contains_type<const int   ,       int &&> == false);
static_assert(contains_type<const int & ,       int &&> == false);
static_assert(contains_type<const int &&,       int &&> == false);
static_assert(contains_type<      int   , const int   > == true);
static_assert(contains_type<      int & , const int   > == true);
static_assert(contains_type<      int &&, const int   > == true);
static_assert(contains_type<const int   , const int   > == true);
static_assert(contains_type<const int & , const int   > == true);
static_assert(contains_type<const int &&, const int   > == true);
static_assert(contains_type<      int   , const int & > == true);
static_assert(contains_type<      int & , const int & > == true);
static_assert(contains_type<      int &&, const int & > == true);
static_assert(contains_type<const int   , const int & > == true);
static_assert(contains_type<const int & , const int & > == true);
static_assert(contains_type<const int &&, const int & > == true);
static_assert(contains_type<      int   , const int &&> == true);
static_assert(contains_type<      int & , const int &&> == false);
static_assert(contains_type<      int &&, const int &&> == true);
static_assert(contains_type<const int   , const int &&> == true);
static_assert(contains_type<const int & , const int &&> == false);
static_assert(contains_type<const int &&, const int &&> == true);

// Straight up different types.
static_assert(contains_type<int, float> == false);

// Trying to copy a non-copyable type.
static_assert(contains_type<std::unique_ptr<int> &, std::unique_ptr<int>> == false);



// --- Adjustment:

struct AA {};
struct BB {AA a; friend constexpr auto &&_adl_em_refl_ReflectAs(int, em::Meta::same_ignoring_cvref<BB> auto &&self) {return EM_FWD(self).a;}};
struct CC {BB b; friend constexpr auto &&_adl_em_refl_ReflectAs(int, em::Meta::same_ignoring_cvref<CC> auto &&self) {return EM_FWD(self).b;}};

static_assert(contains_type<BB &, AA &> == true);
static_assert(contains_type<CC &, AA &> == true); // Repeated adjustment.
static_assert(contains_type<BB &, const AA &> == true);
static_assert(contains_type<BB &&, AA &&> == true);
static_assert(contains_type<const BB &, AA &> == false); // Dropping qualifiers.
static_assert(contains_type<BB &, AA &&> == false); // Dropping qualifiers.



// --- Indirect:

static_assert(contains_type<std::optional<int>, int> == true);
static_assert(contains_type<std::optional<int>, int &> == false);
static_assert(contains_type<std::optional<int> &, int &> == true);
static_assert(contains_type<std::optional<std::optional<int>>, int> == true);

// Optional forwarding:
static_assert(contains_type<std::optional<int> &&, int &&> == true);
static_assert(contains_type<std::optional<int> &&, int &> == false);
static_assert(contains_type<const std::optional<int> &, int &> == false);
static_assert(contains_type<std::shared_ptr<int> &&, int &&> == false);
static_assert(contains_type<std::shared_ptr<int> &&, int &> == true);
static_assert(contains_type<const std::shared_ptr<int> &, int &> == true);



// --- Ranges:

static_assert(contains_type<std::vector<int>, int> == true);
static_assert(contains_type<std::vector<std::vector<int>>, int> == true);
static_assert(contains_type<std::vector<int>, int &> == false);
static_assert(contains_type<std::vector<int> &, int &> == true);
static_assert(contains_type<std::vector<int> &&, int &> == false);
static_assert(contains_type<std::vector<int> &&, int &&> == true); // We do forward the elements for the vector.
static_assert(contains_type<const std::vector<int> &, int &> == false);
static_assert(contains_type<std::vector<int> &, const int &> == true);
static_assert(contains_type<std::vector<int> &&, const int &&> == true);
static_assert(contains_type<std::vector<int> &&, const int &> == true);
static_assert(contains_type<const std::vector<int> &&, const int &> == true);
static_assert(contains_type<const std::vector<int> &&, const int &&> == true);
static_assert(contains_type<const std::vector<int> &&, int &&> == false);

// Span doesn't forward cvref-qualifiers on the elements.
static_assert(contains_type<const std::span<int> &, int &> == true);


// --- Variant

static_assert(contains_type<std::variant<int, float>, int> == true);
static_assert(contains_type<std::variant<int, float>, float> == true);
static_assert(contains_type<std::variant<int, float>, double> == false);

static_assert(contains_type<std::variant<int, float> &, float &> == true);
static_assert(contains_type<std::variant<int, float> &&, float &> == false);
static_assert(contains_type<std::variant<int, float> &&, float &&> == true);

// Variants can't store references, so we're not testing that.



// --- Classes

// Understanding the bases even if not reflected.
struct HiddenBase {};
struct HasHiddenBase : HiddenBase { EM_REFL() };
// Note! This is a discrepancy between `TypeContainsElemCvref` and `VisitTypes`.
static_assert(em::Refl::TypeContainsElemCvref<HasHiddenBase, HiddenBase>);
static_assert(!contains_type_using_visit<HasHiddenBase>(em::Meta::type_to_desc<HiddenBase>));

struct A { EM_REFL() };
struct B { EM_REFL() };
struct C : A { EM_REFL() };
struct D : B { EM_REFL() };
struct E : virtual C, D {};
static_assert(contains_type<E, A> == true);
static_assert(contains_type<E, A> == true);
static_assert(contains_type<E, C> == true);
static_assert(contains_type<E, D> == true);
static_assert(contains_type<E, E> == true);
static_assert(contains_type<E, B &> == false);
static_assert(contains_type<E &, B &> == true);
static_assert(contains_type<E &, B &&> == false);
static_assert(contains_type<E &&, B &&> == true);
static_assert(contains_type<E &&, const B &&> == true);
static_assert(contains_type<const E &&, const B &&> == true);
