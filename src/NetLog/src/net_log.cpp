#include "net_log.h"

#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace net
{

    std::shared_ptr<spdlog::logger> log::s_ServerLogger;
    std::shared_ptr<spdlog::logger> log::s_ConnectionLogger;
    std::shared_ptr<spdlog::logger> log::s_ClientLogger;

    void log::Init()
    {
        spdlog::set_pattern("%^[%T] %n: %v%$");
        s_ServerLogger = spdlog::stdout_color_mt("SERVER LOGGER");
        s_ConnectionLogger = spdlog::stdout_color_mt("CONNECTION LOGGER");
        s_ClientLogger = spdlog::stdout_color_mt("CLIENT LOGGER");
        s_ServerLogger->set_level(spdlog::level::trace);
        s_ConnectionLogger->set_level(spdlog::level::trace);
        s_ClientLogger->set_level(spdlog::level::trace);
    }
}