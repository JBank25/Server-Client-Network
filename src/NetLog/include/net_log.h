#pragma once

#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#define DEBUG_TRACE   4
#define DEBUG_INFO    3
#define DEBUG_WARN    2
#define DEBUG_ERROR   1
#define DEBUG_CRITICAL 0

namespace net
{
    class log
    {

    public:
        inline static std::shared_ptr<spdlog::logger>& GetLogger() { return s_Logger; }
        static void Init();
    private:
        static std::shared_ptr<spdlog::logger> s_Logger;
    };
};

#if LOG_LEVEL >= DEBUG_TRACE
    #define NET_LOG_TRACE(...) log::GetLogger()->trace(__VA_ARGS__);
#else
    #define NET_LOG_TRACE(...) do {} while(0);
#endif

#if LOG_LEVEL >= DEBUG_INFO
    #define NET_LOG_INFO(...) log::GetLogger()->info(__VA_ARGS__);
#else
    #define NET_LOG_INFO(...) do {} while(0);
#endif

#if LOG_LEVEL >= DEBUG_WARN
    #define NET_LOG_WARN(...) log::GetLogger()->warn(__VA_ARGS__);
#else
    #define NET_LOG_WARN(...) do {} while(0);
#endif

#if LOG_LEVEL >= DEBUG_ERROR
    #define NET_LOG_ERROR(...) log::GetLogger()->error(__VA_ARGS__);
#else
    #define NET_LOG_ERROR(...) do {} while(0);
#endif

#if LOG_LEVEL >= DEBUG_CRITICAL
    #define NET_LOG_CRITICAL(...) log::GetLogger()->critical(__VA_ARGS__);
#else
    #define NET_LOG_CRITICAL(...) do {} while(0);
#endif