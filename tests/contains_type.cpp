#include "em/refl/contains_type.h"

#include <memory>
#include <vector>

// Same type with different cvref-qualifiers.
static_assert(em::Refl::TypeContainsElemCvref<      int   ,       int   > == true);
static_assert(em::Refl::TypeContainsElemCvref<      int & ,       int   > == true);
static_assert(em::Refl::TypeContainsElemCvref<      int &&,       int   > == true);
static_assert(em::Refl::TypeContainsElemCvref<const int   ,       int   > == true);
static_assert(em::Refl::TypeContainsElemCvref<const int & ,       int   > == true);
static_assert(em::Refl::TypeContainsElemCvref<const int &&,       int   > == true);
static_assert(em::Refl::TypeContainsElemCvref<      int   ,       int & > == false);
static_assert(em::Refl::TypeContainsElemCvref<      int & ,       int & > == true);
static_assert(em::Refl::TypeContainsElemCvref<      int &&,       int & > == false);
static_assert(em::Refl::TypeContainsElemCvref<const int   ,       int & > == false);
static_assert(em::Refl::TypeContainsElemCvref<const int & ,       int & > == false);
static_assert(em::Refl::TypeContainsElemCvref<const int &&,       int & > == false);
static_assert(em::Refl::TypeContainsElemCvref<      int   ,       int &&> == true);
static_assert(em::Refl::TypeContainsElemCvref<      int & ,       int &&> == false);
static_assert(em::Refl::TypeContainsElemCvref<      int &&,       int &&> == true);
static_assert(em::Refl::TypeContainsElemCvref<const int   ,       int &&> == false);
static_assert(em::Refl::TypeContainsElemCvref<const int & ,       int &&> == false);
static_assert(em::Refl::TypeContainsElemCvref<const int &&,       int &&> == false);
static_assert(em::Refl::TypeContainsElemCvref<      int   , const int   > == true);
static_assert(em::Refl::TypeContainsElemCvref<      int & , const int   > == true);
static_assert(em::Refl::TypeContainsElemCvref<      int &&, const int   > == true);
static_assert(em::Refl::TypeContainsElemCvref<const int   , const int   > == true);
static_assert(em::Refl::TypeContainsElemCvref<const int & , const int   > == true);
static_assert(em::Refl::TypeContainsElemCvref<const int &&, const int   > == true);
static_assert(em::Refl::TypeContainsElemCvref<      int   , const int & > == true);
static_assert(em::Refl::TypeContainsElemCvref<      int & , const int & > == true);
static_assert(em::Refl::TypeContainsElemCvref<      int &&, const int & > == true);
static_assert(em::Refl::TypeContainsElemCvref<const int   , const int & > == true);
static_assert(em::Refl::TypeContainsElemCvref<const int & , const int & > == true);
static_assert(em::Refl::TypeContainsElemCvref<const int &&, const int & > == true);
static_assert(em::Refl::TypeContainsElemCvref<      int   , const int &&> == true);
static_assert(em::Refl::TypeContainsElemCvref<      int & , const int &&> == false);
static_assert(em::Refl::TypeContainsElemCvref<      int &&, const int &&> == true);
static_assert(em::Refl::TypeContainsElemCvref<const int   , const int &&> == true);
static_assert(em::Refl::TypeContainsElemCvref<const int & , const int &&> == false);
static_assert(em::Refl::TypeContainsElemCvref<const int &&, const int &&> == true);

// Straight up different types.
static_assert(em::Refl::TypeContainsElemCvref<int, float> == false);

// Trying to copy a non-copyable type.
static_assert(em::Refl::TypeContainsElemCvref<std::unique_ptr<int> &, std::unique_ptr<int>> == false);



// --- Adjustment:

struct A {};
struct B {A a; friend constexpr auto &&_adl_em_refl_ReflectAs(int, em::Meta::same_ignoring_cvref<B> auto &&self) {return EM_FWD(self).a;}};
struct C {B b; friend constexpr auto &&_adl_em_refl_ReflectAs(int, em::Meta::same_ignoring_cvref<C> auto &&self) {return EM_FWD(self).b;}};

static_assert(em::Refl::TypeContainsElemCvref<B &, A &> == true);
static_assert(em::Refl::TypeContainsElemCvref<C &, A &> == true); // Repeated adjustment.
static_assert(em::Refl::TypeContainsElemCvref<B &, const A &> == true);
static_assert(em::Refl::TypeContainsElemCvref<B &&, A &&> == true);
static_assert(em::Refl::TypeContainsElemCvref<const B &, A &> == false); // Dropping qualifiers.
static_assert(em::Refl::TypeContainsElemCvref<B &, A &&> == false); // Dropping qualifiers.



// --- Indirect:

static_assert(em::Refl::TypeContainsElemCvref<std::optional<int>, int> == true);
static_assert(em::Refl::TypeContainsElemCvref<std::optional<int>, int &> == false);
static_assert(em::Refl::TypeContainsElemCvref<std::optional<int> &, int &> == true);
static_assert(em::Refl::TypeContainsElemCvref<std::optional<std::optional<int>>, int> == true);

// Optional forwarding:
static_assert(em::Refl::TypeContainsElemCvref<std::optional<int> &&, int &&> == true);
static_assert(em::Refl::TypeContainsElemCvref<std::optional<int> &&, int &> == false);
static_assert(em::Refl::TypeContainsElemCvref<const std::optional<int> &, int &> == false);
static_assert(em::Refl::TypeContainsElemCvref<std::shared_ptr<int> &&, int &&> == false);
static_assert(em::Refl::TypeContainsElemCvref<std::shared_ptr<int> &&, int &> == true);
static_assert(em::Refl::TypeContainsElemCvref<const std::shared_ptr<int> &, int &> == true);



// --- Ranges:

static_assert(em::Refl::TypeContainsElemCvref<std::vector<int>, int> == true);
static_assert(em::Refl::TypeContainsElemCvref<std::vector<std::vector<int>>, int> == true);
static_assert(em::Refl::TypeContainsElemCvref<std::vector<int>, int &> == false);
static_assert(em::Refl::TypeContainsElemCvref<std::vector<int> &, int &> == true);
static_assert(em::Refl::TypeContainsElemCvref<std::vector<int> &&, int &> == false);
static_assert(em::Refl::TypeContainsElemCvref<std::vector<int> &&, int &&> == true); // We do forward the elements for the vector.
static_assert(em::Refl::TypeContainsElemCvref<const std::vector<int> &, int &> == false);
static_assert(em::Refl::TypeContainsElemCvref<std::vector<int> &, const int &> == true);
static_assert(em::Refl::TypeContainsElemCvref<std::vector<int> &&, const int &&> == true);
static_assert(em::Refl::TypeContainsElemCvref<std::vector<int> &&, const int &> == true);
static_assert(em::Refl::TypeContainsElemCvref<const std::vector<int> &&, const int &> == true);
static_assert(em::Refl::TypeContainsElemCvref<const std::vector<int> &&, const int &&> == true);
static_assert(em::Refl::TypeContainsElemCvref<const std::vector<int> &&, int &&> == false);

// Span doesn't forward cvref-qualifiers on the elements.
static_assert(em::Refl::TypeContainsElemCvref<const std::span<int> &, int &> == true);



// --- Variant

static_assert(em::Refl::TypeContainsElemCvref<std::variant<int, float>, int> == true);
static_assert(em::Refl::TypeContainsElemCvref<std::variant<int, float>, float> == true);
static_assert(em::Refl::TypeContainsElemCvref<std::variant<int, float>, double> == false);

static_assert(em::Refl::TypeContainsElemCvref<std::variant<int, float> &, float &> == true);
static_assert(em::Refl::TypeContainsElemCvref<std::variant<int, float> &&, float &> == false);
static_assert(em::Refl::TypeContainsElemCvref<std::variant<int, float> &&, float &&> == true);

// Variants can't store references, so we're not testing that.
