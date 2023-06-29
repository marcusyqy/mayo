#pragma once

#ifdef ZOO_ENABLE_LOGS
#include <spdlog/spdlog.h>

#ifdef ZOO_VERBOSE_LOGS
#define ZOO_LOG_INFO(...)                                                      \
    spdlog::info("[{} in \".{}-{}\"] : {}", __FUNCTION__, __FILE__, __LINE__,  \
        fmt::format(__VA_ARGS__))
#define ZOO_LOG_ERROR(...)                                                     \
    spdlog::error("[{} in \".{}-{}\"] : {}", __FUNCTION__, __FILE__, __LINE__, \
        fmt::format(__VA_ARGS__))
#define ZOO_LOG_TRACE(...)                                                     \
    spdlog::trace("[{} in \".{}-{}\"] : {}", __FUNCTION__, __FILE__, __LINE__, \
        fmt::format(__VA_ARGS__))
#define ZOO_LOG_WARN(...)                                                      \
    spdlog::warn("[{} in \".{}-{}\"] : {}", __FUNCTION__, __FILE__, __LINE__,  \
        fmt::format(__VA_ARGS__))

#else
#define ZOO_LOG_INFO(...)  spdlog::info(__VA_ARGS__)
#define ZOO_LOG_ERROR(...) spdlog::error(__VA_ARGS__)
#define ZOO_LOG_TRACE(...) spdlog::trace(__VA_ARGS__)
#define ZOO_LOG_WARN(...)  spdlog::warn(__VA_ARGS__)
#endif

#else
#define ZOO_LOG_INFO(...)
#define ZOO_LOG_ERROR(...)
#define ZOO_LOG_TRACE(...)
#define ZOO_LOG_WARN(...)

#endif
