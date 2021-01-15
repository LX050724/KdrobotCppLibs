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
    QCommandLineOption noUdp("noUDP", "Disable Udp radio");
    QCommandLineOption log({"l", "log"}, "Set the log file path. default disable", "log");
    QCommandLineOption TcpPort({"t", "TcpPort"}, "Set Tcp Port, the default is 8550", "TcpPort");
    TcpPort.setDefaultValue("8550");

    parser.addHelpOption();
    parser.addOptions({noUdp, log, TcpPort});
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

    uint16_t port = 8550;
    if (parser.isSet(noUdp))
        logger.info("no Udp mod");

    if (parser.isSet(TcpPort)) {
        QString portString = parser.value(TcpPort);
        bool Ok;
        uint16_t t = portString.toUShort(&Ok);
        if (Ok) {
            port = t;
            logger.info("set Tcp port {}", port);
        } else {
            logger.error("Tcp port input error");
            return 0;
        }
    }

    RCS_Server *server;
    try {
        server = new RCS_Server(port, !parser.isSet(noUdp));
    } catch (const std::runtime_error &e) {
        logger.error(e.what());
        return -1;
    }
    return a.exec();
}