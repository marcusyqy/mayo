#pragma once
#include "types.hpp"

struct String_View : Buffer_View<const char> {
    using underlying = Buffer_View<const char>;

    String_View(const char* str);
    String_View(const char* str, size_t size);
};

String_View to_string_view(const char* str);

struct String {};
