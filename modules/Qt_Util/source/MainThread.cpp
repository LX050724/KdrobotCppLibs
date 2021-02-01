#include "MainThread.h"

MainThread::MainThread(const QStringList &args, QObject *parent) : QThread(parent), logger("main") {
    this->args = args;
    QCommandLineOption log({"l", "log"}, "Set the log file path. default disable", "log");
    QCommandLineOption conf({"c", "config"}, "set config file path", "config", "config.json");
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addOptions({log, conf});
    parser.process(args);
    spdlogger l("MainThread");
    QString logFile = parser.value("log");
    if (!logFile.isEmpty()) {
        spdlogger::allLogger_logToFile(logFile.toStdString());
        logger.LogToFile();
        l.info("log to file '{}'", logFile);
    }
    l.info("Build Time: {} {}", __DATE__, __TIME__);
#if defined(__DEBUG__)
    spdlog::set_level(spdlog::level::debug);
    l.info("Build type: Debug");
#else
    l.info("Build type: Release");
#endif
    QString configFile = parser.value("config");
    config = JsonConfig::factory(configFile);
    if (config.isOpen()) {
        l.info("open the config file'{}'", configFile.toStdString());
        if (logFile.isEmpty()) {
            auto logPath = config.findObject("log").toString();
            if (!logPath.isEmpty()) {
                l.info("log to file '{}'", logPath);
                spdlogger::allLogger_logToFile(logPath.toStdString());
                logger.LogToFile();
            }
        }
    } else l.warn("can't open the config file'{}'", configFile.toStdString());
    l.flush();
    this->start();
}

void MainThread::run() {
    logger.info("Thread start");
    main(args);
    spdlog::apply_all([&](std::shared_ptr<spdlog::logger> l) { l->flush(); });
    logger.flush();
    emit finished();
}

MainThread::~MainThread() {
    if (this->isRunning() && running) {
        running = false;
        this->quit();
        this->wait();
    }
}
