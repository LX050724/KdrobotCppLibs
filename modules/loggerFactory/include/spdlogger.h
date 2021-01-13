#ifndef KDROBOTCPPLIBS_SPDLOGGER_H
#define KDROBOTCPPLIBS_SPDLOGGER_H

#include <utility>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <QtCompatibilityLayer.h>

class spdlogger {
    std::shared_ptr<spdlog::logger> log, err_log;
public:
    spdlogger(std::shared_ptr<spdlog::logger> _logger, std::shared_ptr<spdlog::logger> _err_logger) :
            log(_logger), err_log(_err_logger) {}

    spdlogger(const spdlogger &copy) :
            log(copy.log), err_log(copy.err_log) {}

    spdlogger(const spdlogger &&mov) :
            log(std::move(mov.log)), err_log(std::move(mov.err_log)) {}

    spdlogger(const char *name);

    spdlogger operator=(const spdlogger &logger) {
        this->log = logger.log;
        this->err_log = logger.err_log;
        return *this;
    }

    template<typename T>
    void trace(const T &msg) {
        log->trace(msg);
    }

    template<typename T>
    void debug(const T &msg) {
        log->debug(msg);
    }

    template<typename T>
    void info(const T &msg) {
        log->info(msg);
    }

    template<typename T>
    void warn(const T &msg) {
        log->warn(msg);
    }

    template<typename T>
    void error(const T &msg) {
        err_log->error(msg);
    }

    template<typename T>
    void critical(const T &msg) {
        log->critical(msg);
    }

    template<typename FormatString, typename... Args>
    void trace(const FormatString &fmt, const Args &... args) {
        log->trace(fmt, args...);
    }

    template<typename FormatString, typename... Args>
    void debug(const FormatString &fmt, const Args &... args) {
        log->debug(fmt, args...);
    }

    template<typename FormatString, typename... Args>
    void info(const FormatString &fmt, const Args &... args) {
        log->info(fmt, args...);
    }

    template<typename FormatString, typename... Args>
    void warn(const FormatString &fmt, const Args &... args) {
        log->warn(fmt, args...);
    }

    template<typename FormatString, typename... Args>
    void error(const FormatString &fmt, const Args &... args) {
        err_log->error(fmt, args...);
    }

    template<typename FormatString, typename... Args>
    void critical(const FormatString &fmt, const Args &... args) {
        log->critical(fmt, args...);
    }

    static spdlogger getLogger(const char *name);
};


#endif
