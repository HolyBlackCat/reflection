#include "em/refl/access/ranges.h"

#include <span>
#include <vector>

static_assert(em::Refl::Ranges::TypeUnqualified<std::vector<int>>);
static_assert(!em::Refl::Ranges::TypeUnqualified<const std::vector<int>>);
static_assert(em::Refl::Ranges::Type<std::vector<int>>);
static_assert(em::Refl::Ranges::Type<const std::vector<int>>);
static_assert(em::Refl::Ranges::Type<const volatile std::vector<int> &&>);

static_assert(em::Refl::Ranges::ShouldForwardElements<std::vector<int>>);
static_assert(!em::Refl::Ranges::ShouldForwardElements<std::span<int>>);

static_assert(std::is_same_v<em::Refl::Ranges::ElementType<std::vector<int>>, int>);
static_assert(std::is_same_v<em::Refl::Ranges::ElementType<const volatile std::vector<int> &&>, int>);
static_assert(std::is_same_v<em::Refl::Ranges::ElementType<std::span<int>>, int>);
static_assert(std::is_same_v<em::Refl::Ranges::ElementType<const volatile std::span<int> &&>, int>);

static_assert(std::is_same_v<em::Refl::Ranges::ElementTypeCvref<      std::vector<int>   >,       int &&>);
static_assert(std::is_same_v<em::Refl::Ranges::ElementTypeCvref<const std::vector<int>   >, const int &&>);
static_assert(std::is_same_v<em::Refl::Ranges::ElementTypeCvref<      std::vector<int> & >,       int & >);
static_assert(std::is_same_v<em::Refl::Ranges::ElementTypeCvref<const std::vector<int> & >, const int & >);
static_assert(std::is_same_v<em::Refl::Ranges::ElementTypeCvref<      std::vector<int> &&>,       int &&>);
static_assert(std::is_same_v<em::Refl::Ranges::ElementTypeCvref<const std::vector<int> &&>, const int &&>);

static_assert(std::is_same_v<em::Refl::Ranges::ElementTypeCvref<      std::span<int>   >, int &>);
static_assert(std::is_same_v<em::Refl::Ranges::ElementTypeCvref<const std::span<int>   >, int &>);
static_assert(std::is_same_v<em::Refl::Ranges::ElementTypeCvref<      std::span<int> & >, int &>);
static_assert(std::is_same_v<em::Refl::Ranges::ElementTypeCvref<const std::span<int> & >, int &>);
static_assert(std::is_same_v<em::Refl::Ranges::ElementTypeCvref<      std::span<int> &&>, int &>);
static_assert(std::is_same_v<em::Refl::Ranges::ElementTypeCvref<const std::span<int> &&>, int &>);

static int a[3]{};
[[maybe_unused]] static auto zip = std::views::enumerate(a);
using Zip = decltype(zip);
static_assert(em::Refl::Ranges::Type<Zip>);
static_assert(!em::Refl::Ranges::ShouldForwardElements<Zip>);
static_assert(std::is_same_v<em::Refl::Ranges::ElementType<Zip>, std::tuple<std::ptrdiff_t, int>>);
static_assert(std::is_same_v<em::Refl::Ranges::ElementType<const volatile Zip &&>, std::tuple<std::ptrdiff_t, int>>);
static_assert(std::is_same_v<em::Refl::Ranges::ElementTypeCvref<      Zip   >, std::tuple<std::ptrdiff_t, int &>>); // `views::zip` doesn't own the elements, so there is no forwarding here.
static_assert(std::is_same_v<em::Refl::Ranges::ElementTypeCvref<const Zip   >, std::tuple<std::ptrdiff_t, int &>>);
static_assert(std::is_same_v<em::Refl::Ranges::ElementTypeCvref<      Zip & >, std::tuple<std::ptrdiff_t, int &>>);
static_assert(std::is_same_v<em::Refl::Ranges::ElementTypeCvref<const Zip & >, std::tuple<std::ptrdiff_t, int &>>);
static_assert(std::is_same_v<em::Refl::Ranges::ElementTypeCvref<      Zip &&>, std::tuple<std::ptrdiff_t, int &>>);
static_assert(std::is_same_v<em::Refl::Ranges::ElementTypeCvref<const Zip &&>, std::tuple<std::ptrdiff_t, int &>>);
