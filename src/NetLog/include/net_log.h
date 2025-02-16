#pragma once

#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

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

// TODO: build switch these out if doing a production build meant to be distributed
// #ifdef(PDOUCTION_BUILD)
//      #define NET_LOG_TRACE(...)
// #endif
// Something like this^
#define NET_LOG_TRACE(...)      log::GetLogger()->trace(__VA_ARGS__);
#define NET_LOG_INFO(...)       log::GetLogger()->info(__VA_ARGS__);
#define NET_LOG_WARN(...)       log::GetLogger()->warn(__VA_ARGS__);
#define NET_LOG_ERROR(...)      log::GetLogger()->error(__VA_ARGS__);
#define NET_LOG_CRITICAL(...)   log::GetLogger()->critical(__VA_ARGS__);