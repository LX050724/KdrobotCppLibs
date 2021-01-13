#include "spdlogger.h"

spdlogger spdlogger::getLogger(const char *name) {
    if (spdlog::get(name) == nullptr) {
        auto &&logger = spdlog::stdout_color_mt(name);
        auto &&err_logger = spdlog::stderr_color_mt(std::string(name) + "_error");
        return {logger, err_logger};
    } else {
        auto &&logger = spdlog::get(name);
        auto &&err_logger = spdlog::get(std::string(name) + "_error");
        return {logger, err_logger};
    }
}

spdlogger::spdlogger(const char *name) {
    spdlogger t = spdlogger::getLogger(name);
    log = t.log;
    err_log = t.err_log;
}
