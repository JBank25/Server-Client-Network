#include "net_log.h"

#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace net
{

    std::shared_ptr<spdlog::logger> log::s_ServerLogger;
    std::shared_ptr<spdlog::logger> log::s_ConnectionLogger;

    // template <>
    // struct fmt::formatter<my_type> : fmt::formatter<std::string> {
        // auto format(my_type my, format_context &ctx) const -> decltype(ctx.out()) {
            // return fmt::format_to(ctx.out(), "[my_type i={}]", my.i);
        // }
    // };

    void log::Init()
    {
        spdlog::set_pattern("%^[%T] %n: %v%$");
        s_ServerLogger = spdlog::stdout_color_mt("SERVER LOGGER");
        s_ConnectionLogger = spdlog::stdout_color_mt("CONNECTION LOGGER");
        s_ServerLogger->set_level(spdlog::level::trace);
        s_ConnectionLogger->set_level(spdlog::level::trace);
    }
}