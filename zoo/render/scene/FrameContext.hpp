#pragma once
#include "render/fwd.hpp"
#include "render/resources/PoolAllocator.hpp"

namespace zoo::render::scene {

class FrameContext {
public:
private:
    resources::PoolAllocator allocator_;
};

} // namespace zoo::render::scene
