#include "fwd.hpp"

namespace zoo::render::hidden::detail {

void maybe_invoke(VkResult result) noexcept { std::exit(result); }

} // namespace zoo::render::hidden::detail
