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