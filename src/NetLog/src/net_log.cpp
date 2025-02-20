#include "net_log.h"

#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace net
{

    std::shared_ptr<spdlog::logger> log::s_ServerLogger;
    std::shared_ptr<spdlog::logger> log::s_ConnectionLogger;

    void log::Init()
    {
        spdlog::set_pattern("%^[%T] %n: %v%$");
        s_ServerLogger = spdlog::stdout_color_mt("SERVER LOGGER");
        s_ServerLogger = spdlog::stdout_color_mt("SERVER LOGGER");
        s_ConnectionLogger->set_level(spdlog::level::trace);
        s_ConnectionLogger->set_level(spdlog::level::trace);
    }
}