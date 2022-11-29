#pragma once

#include "../stdx/enum.hpp"

namespace refl {

enum class type_attr {
    is_none = 0x00,
    is_constant = 0x01,
    is_mut = 0x02,
    is_pointer = 0x03,
    is_reference = 0x04,
    is_class = 0x05,
    _stdx_enum_as_bitmask
};

}
