/**
 * @file main.cpp
 * @author yao
 * @date 2021年1月13日
 * @brief 机器人网络通讯系统服务器主函数
 */

#include <QCoreApplication>
#include <spdlog/spdlog.h>
#include <RCS_Server.h>
#include <spdlogger.h>

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption({{"l", "log"}, "设置日志文件路径，默认不开启", "log"});
    parser.process(a);
    QString logFile = parser.value("log");
    if (!logFile.isEmpty()) {
        spdlogger::allLogger_logToFile(logFile.toStdString());
    }
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [thread %t] [%^%-8l%$]: %v");
    spdlogger logger(__FUNCTION__);
    logger.info("Build Time: {} {}", __DATE__, __TIME__);

#if defined(__DEBUG__)
    spdlog::set_level(spdlog::level::debug);
    logger.info("Build type: Debug");
#else
    logger.info("Build type: Release");
#endif
    RCS_Server server;
    return a.exec();
}