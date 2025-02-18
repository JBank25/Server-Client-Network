#include "net_log.h"

#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace net
{

    std::shared_ptr<spdlog::logger> log::s_Logger;

    void log::Init()
    {
        spdlog::set_pattern("%^[%T] %n: %v%$");
        s_Logger = spdlog::stdout_color_mt("LOGGER");
        s_Logger->set_level(spdlog::level::trace);
    }
}