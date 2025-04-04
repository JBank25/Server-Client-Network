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
        inline static std::shared_ptr<spdlog::logger>& GetServerLogger() { return s_ServerLogger; }
        inline static std::shared_ptr<spdlog::logger>& GetConnectionLogger() { return s_ConnectionLogger; }
        inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
        static void Init();
    private:
        static std::shared_ptr<spdlog::logger> s_ServerLogger;
        static std::shared_ptr<spdlog::logger> s_ClientLogger;
        static std::shared_ptr<spdlog::logger> s_ConnectionLogger;
    };
};

#if LOG_LEVEL >= DEBUG_TRACE
    #define NET_SERVER_LOG_TRACE(...)       log::GetServerLogger()->trace(__VA_ARGS__);
    #define NET_CONNECTION_LOG_TRACE(...)   log::GetConnectionLogger()->trace(__VA_ARGS__);
    #define NET_CLIENT_LOG_TRACE(...)   log::GetClientLogger()->trace(__VA_ARGS__);
#else
    #define NET_SERVER_LOG_TRACE(...)       do {} while(0);
    #define NET_CONNECTION_LOG_TRACE(...)   do {} while(0);
    #define NET_CLIENT_LOG_TRACE(...)   do {} while(0);
#endif

#if LOG_LEVEL >= DEBUG_INFO
    #define NET_SERVER_LOG_INFO(...)        log::GetServerLogger()->info(__VA_ARGS__);
    #define NET_CONNECTION_LOG_INFO(...)    log::GetConnectionLogger()->info(__VA_ARGS__);
    #define NET_CLIENT_LOG_INFO(...)    log::GetConnectionLogger()->info(__VA_ARGS__);
#else
    #define NET_SERVER_LOG_INFO(...)        do {} while(0);
    #define NET_CONNECTION_LOG_INFO(...)    do {} while(0);
    #define NET_CLIENT_LOG_INFO(...)    do {} while(0);
#endif

#if LOG_LEVEL >= DEBUG_WARN
    #define NET_SERVER_LOG_WARN(...)        log::GetServerLogger()->warn(__VA_ARGS__);
    #define NET_CONNECTION_LOG_WARN(...)    log::GetConnectionLogger()->warn(__VA_ARGS__);
    #define NET_CLIENT_LOG_WARN(...)    log::GetConnectionLogger()->warn(__VA_ARGS__);
#else
    #define NET_SERVER_LOG_WARN(...)        do {} while(0);
    #define NET_CONNECTION_LOG_WARN(...)    do {} while(0);
    #define NET_CLIENT_LOG_WARN(...)    do {} while(0);
#endif

#if LOG_LEVEL >= DEBUG_ERROR
    #define NET_SERVER_LOG_ERROR(...)       log::GetServerLogger()->error(__VA_ARGS__);
    #define NET_CONNECTION_LOG_ERROR(...)   log::GetConnectionLogger()->error(__VA_ARGS__);
    #define NET_CLIENT_LOG_ERROR(...)   log::GetConnectionLogger()->error(__VA_ARGS__);
#else
    #define NET_SERVER_LOG_ERROR(...)       do {} while(0);
    #define NET_CONNECTION_LOG_ERROR(...)   do {} while(0);
    #define NET_CLIENT_LOG_ERROR(...)   do {} while(0);
#endif

#if LOG_LEVEL >= DEBUG_CRITICAL
    #define NET_SERVER_LOG_CRITICAL(...)        log::GetServerLogger()->critical(__VA_ARGS__);
    #define NET_CONNECTION_LOG_CRITICAL(...)    log::GetConnectionLogger()->critical(__VA_ARGS__);
    #define NET_CLIENT_LOG_CRITICAL(...)    log::GetConnectionLogger()->critical(__VA_ARGS__);
#else
    #define NET_SERVER_LOG_CRITICAL(...)        do {} while(0);
    #define NET_CONNECTION_LOG_CRITICAL(...)    do {} while(0);
    #define NET_CLIENT_LOG_CRITICAL(...)    do {} while(0);
#endif