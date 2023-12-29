#pragma once

#ifdef SIMP_ENABLE_LOGS
#include <spdlog/spdlog.h>

namespace detail {
#define logger_init() ::spdlog::set_level(::spdlog::level::trace)
}

#ifdef _VERBOSE_LOGS
#define log_info(...)                                                                                                  \
    spdlog::info("[{} in \".{}-{}\"] : {}", __FUNCTION__, __FILE__, __LINE__, fmt::format(__VA_ARGS__))
#define log_error(...)                                                                                                 \
    spdlog::error("[{} in \".{}-{}\"] : {}", __FUNCTION__, __FILE__, __LINE__, fmt::format(__VA_ARGS__))
#define log_trace(...)                                                                                                 \
    spdlog::trace("[{} in \".{}-{}\"] : {}", __FUNCTION__, __FILE__, __LINE__, fmt::format(__VA_ARGS__))
#define log_warn(...)                                                                                                  \
    spdlog::warn("[{} in \".{}-{}\"] : {}", __FUNCTION__, __FILE__, __LINE__, fmt::format(__VA_ARGS__))
#define log_debug(...)                                                                                                  \
    spdlog::debug("[{} in \".{}-{}\"] : {}", __FUNCTION__, __FILE__, __LINE__, fmt::format(__VA_ARGS__))

#else
#define log_info(...)  spdlog::info(__VA_ARGS__)
#define log_error(...) spdlog::error(__VA_ARGS__)
#define log_trace(...) spdlog::trace(__VA_ARGS__)
#define log_warn(...)  spdlog::warn(__VA_ARGS__)
#define log_debug(...)  spdlog::debug(__VA_ARGS__)
#endif

#else
#define logger_init()
#define log_info(...)
#define log_error(...)
#define log_trace(...)
#define log_warn(...)
#define log_debug(...)

#endif
