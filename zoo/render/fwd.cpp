#include "fwd.hpp"

namespace zoo::render {

namespace hidden::detail {

void maybe_invoke(VkResult result) noexcept { std::exit(result); }

} // namespace hidden::detail
} // namespace zoo::render
