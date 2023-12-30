#include "string.hpp"
#include <cstring>

String_View::String_View(const char* str) : underlying{ str, strlen(str) } {}
String_View::String_View(const char* str, size_t size) : underlying{ str, size } {}

String_View to_string_view(const char* str) { return { str }; }
