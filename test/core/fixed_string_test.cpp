#include <gtest/gtest.h>
#include "fixed_string.hpp"

template<lzh::fixed_string T>
bool compile_this() noexcept {
    return true;
}

template<lzh::fixed_string T>
class compile_class : public std::true_type {};

// Demonstrate some basic assertions.
TEST(FixedString, BasicCompile) {
    // Expect two strings not to be equal.
    EXPECT_TRUE(compile_this<"hello world">());
    EXPECT_TRUE(compile_class<"hello world">::value);
}
