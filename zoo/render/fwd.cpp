#include "fwd.hpp"

namespace zoo::render::hidden::detail {

uintptr_t align_forward(uintptr_t ptr, size_t align) {
    uintptr_t p, a, modulo;

    ZOO_ASSERT(is_power_of_two(align));

    p = ptr;
    a = (uintptr_t)align;
    // Same as (p % a) but faster as 'a' is a power of two
    modulo = p & (a - 1);

    if (modulo != 0) {
        // If 'p' address is not aligned, push the address to the
        // next value which is aligned
        p += a - modulo;
    }
    return p;
}
void maybe_invoke(VkResult result) noexcept { std::exit(result); }

} // namespace zoo::render::hidden::detail
