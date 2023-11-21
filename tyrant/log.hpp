#pragma once

#ifdef SIMP_ENABLE_LOGS
#include <spdlog/spdlog.h>

#ifdef SIMP_VERBOSE_LOGS
#define LOG_INFO(...)                                                                                                  \
    spdlog::info("[{} in \".{}-{}\"] : {}", __FUNCTION__, __FILE__, __LINE__, fmt::format(__VA_ARGS__))
#define LOG_ERROR(...)                                                                                                 \
    spdlog::error("[{} in \".{}-{}\"] : {}", __FUNCTION__, __FILE__, __LINE__, fmt::format(__VA_ARGS__))
#define LOG_TRACE(...)                                                                                                 \
    spdlog::trace("[{} in \".{}-{}\"] : {}", __FUNCTION__, __FILE__, __LINE__, fmt::format(__VA_ARGS__))
#define LOG_WARN(...)                                                                                                  \
    spdlog::warn("[{} in \".{}-{}\"] : {}", __FUNCTION__, __FILE__, __LINE__, fmt::format(__VA_ARGS__))

#else
#define LOG_INFO(...)  spdlog::info(__VA_ARGS__)
#define LOG_ERROR(...) spdlog::error(__VA_ARGS__)
#define LOG_TRACE(...) spdlog::trace(__VA_ARGS__)
#define LOG_WARN(...)  spdlog::warn(__VA_ARGS__)
#endif

#else
#define LOG_INFO(...)
#define LOG_ERROR(...)
#define LOG_TRACE(...)
#define LOG_WARN(...)

#endif
