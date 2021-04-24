/**
 * @file MainThread.cpp
 * @author yao
 * @date 2021年1月13日
 */

#include "MainThread.h"

using namespace spdlog::level;

MainThread::MainThread(const QStringList &args, QObject *parent) : QThread(parent), logger("main") {
    this->args = args;
    QCommandLineOption currentPath({"d", "directory"}, "Set the working directory", "currentPath");
    QCommandLineOption log({"l", "log"}, "Set the log file path. default disable", "log");
    QCommandLineOption conf({"c", "config"}, "set config file path", "config", "config.json");
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOptions({currentPath, log, conf});
    parser.process(args);

    spdlogger l("MainThread");
    QString configFile = parser.value(conf);
    QString logFile = parser.value(log);
    QString path = parser.value(currentPath);
    config.open(configFile);
    if (path.isEmpty() && config.isOpen()) path = config.getPath();

    if (config.isOpen()) {
        l.info("open the config file'{}'", configFile);
        auto confLog = config.findObject("logger/OutputFile").toString();

        level_enum LogLevel = from_str(config.findObject("logger/LogLevel").toString().toStdString());
        if (LogLevel < n_levels && LogLevel > 0) {
            spdlog::set_level(LogLevel);
            l.info("set log level '{}'", to_string_view(LogLevel));
        } else l.error("'LogLevel' Parameter is invalid");

        if (logFile.isEmpty() && !confLog.isEmpty()) {
            logFile = confLog;
        }
    } else l.warn("can't open the config file'{}'", configFile);

    if (!path.isEmpty()) {
        if (!QDir::setCurrent(path)) l.warn("can not set current path '{}'", path);
        else l.info("set current path '{}'", path);
    } else l.info("current path '{}'", QDir::currentPath());

    if (!logFile.isEmpty()) {
        l.info("log to file '{}'", logFile);
        spdlogger::allLogger_logToFile(logFile.toStdString());
    }
    l.flush();
}

void MainThread::run() {
    logger.info("Thread start");
    main(args);
    spdlog::apply_all([&](std::shared_ptr<spdlog::logger> l) { l->flush(); });
    running = false;
    logger.flush();
    emit threadExit();
}

MainThread::~MainThread() {
    if (this->isRunning() && running) {
        running = false;
        this->quit();
        this->wait();
    }
}
