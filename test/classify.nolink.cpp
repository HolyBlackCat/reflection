#include "em/refl/classify.h"

#include <filesystem>

static_assert(em::Refl::classify_opt<std::filesystem::path> == em::Refl::Category::unknown);
