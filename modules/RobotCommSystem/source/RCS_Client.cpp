/**
 * @file RCS_Client.cpp
 * @author yao
 * @date 2021年1月13日
 */

#include <QNetworkInterface>
#include <QNetworkDatagram>
#include "RCS_Client.h"

RCS_Client::RCS_Client(const QString &_ClientName, uint16_t _TcpPort, uint16_t _UdpPort, QObject *parent) :
        QObject(parent), logger(__FUNCTION__), ClientName(_ClientName) {
    TcpPort = _TcpPort;
    if (ClientName.isEmpty()) {
        logger.error("ClientName is empty");
        throw std::runtime_error("ClientName is empty");
    }
    udpSocket = new QUdpSocket(this);
    QNetworkInterface networkInterface;
    logger.info("HostAddress:");
    for (const QNetworkInterface &anInterface : networkInterface.allInterfaces()) {
        logger.info("\tNetwork Interface Name:{}", anInterface.humanReadableName());
        for (const QNetworkAddressEntry &addressEntry : anInterface.addressEntries()) {
            logger.info("\t\tIP:{} MASK:{}", addressEntry.ip(), addressEntry.netmask());
            if (!addressEntry.ip().isLoopback() && addressEntry.ip().toIPv4Address() != 0) {
                HostAddressEntry.append(addressEntry);
            }
        }
    }
    udpSocket->bind(_UdpPort, QUdpSocket::ShareAddress);
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(UdpReadyRead()));
    waitMutex.lock();
}

RCS_Client::RCS_Client(const QString &_ClientName, const QHostAddress &addr, uint16_t _TcpPort, QObject *parent)
        : logger(__FUNCTION__) {
    logger.info("Custom IP:{} Port:{}", addr, _TcpPort);
    TcpPort = _TcpPort;
    ClientName = _ClientName;
    QTcpSocket *tcpSocket = new QTcpSocket;
    tcpSocket->connectToHost(addr, TcpPort);
    waitMutex.lock();
    if (tcpSocket->waitForConnected(5000)) {
        pTcpConnect = new TcpConnect(tcpSocket, ClientName);

        connect(pTcpConnect,
                SIGNAL(ClientReceive_GET(const QString &, const QString &, const QJsonObject &)),
                this, SLOT(receive_GET(const QString &, const QString &, const QJsonObject &)));

        connect(pTcpConnect,
                SIGNAL(ClientReceive_PUSH(const QString &, const QString &, const QJsonObject &)),
                this, SLOT(receive_PUSH(const QString &, const QString &, const QJsonObject &)));

        connect(pTcpConnect,
                SIGNAL(Receive_BROADCAST(const QString &, const QString &, const QJsonObject &)),
                this, SLOT(receive_BROADCAST(const QString &, const QString &, const QJsonObject &)));

        connect(pTcpConnect,
                SIGNAL(ClientReceive_CLIENT_RET(const QString &, const QJsonObject &)),
                this, SLOT(receive_CLIENT_RET(const QString &, const QJsonObject &)));

        connect(pTcpConnect,
                SIGNAL(ClientReceive_SERVER_RET(const QJsonObject &)),
                this, SLOT(receive_SERVER_RET(const QJsonObject &)));
        Connected = true;
        waitCondition.wakeAll();
        waitMutex.unlock();
    } else {
        logger.error("Tcp Connect Time Out");
        throw std::runtime_error("Tcp Connect Time Out");
    }
}

void RCS_Client::UdpReadyRead() {
    QMutexLocker locker(&udpMutex);
    QNetworkDatagram datagram = udpSocket->receiveDatagram();
    if (Connected)
        return;
    QJsonParseError err;
    QJsonDocument jdom = QJsonDocument::fromJson(datagram.data(), &err);
    if (err.error == QJsonParseError::NoError) {
        QJsonArray remoteAddrEntryList = jdom.object().find("hostAddress")->toArray();
        for (const auto &remoteAddrEntry : remoteAddrEntryList) {
            QJsonObject EntryObject = remoteAddrEntry.toObject();
            QHostAddress remoteIP(EntryObject.find("IP")->toString());
            QHostAddress remoteMASK(EntryObject.find("MASK")->toString());
            for (const auto &localAddr : HostAddressEntry) {
                uint32_t mask = localAddr.netmask().toIPv4Address();
                uint32_t ip = localAddr.ip().toIPv4Address();
                uint32_t host = ip & mask;
                if (remoteMASK.toIPv4Address() == mask && (remoteIP.toIPv4Address() & mask) == host) {
                    logger.info("ip {}", remoteIP.toString());
                    QTcpSocket *tcpSocket = new QTcpSocket;
                    tcpSocket->connectToHost(remoteIP, TcpPort);
                    if (tcpSocket->waitForConnected(5000)) {
                        pTcpConnect = new TcpConnect(tcpSocket, ClientName);

                        connect(pTcpConnect,
                                SIGNAL(ClientReceive_GET(const QString &, const QString &, const QJsonObject &)),
                                this, SLOT(receive_GET(const QString &, const QString &, const QJsonObject &)));

                        connect(pTcpConnect,
                                SIGNAL(ClientReceive_PUSH(const QString &, const QString &, const QJsonObject &)),
                                this, SLOT(receive_PUSH(const QString &, const QString &, const QJsonObject &)));

                        connect(pTcpConnect,
                                SIGNAL(Receive_BROADCAST(const QString &, const QString &, const QJsonObject &)),
                                this, SLOT(receive_BROADCAST(const QString &, const QString &, const QJsonObject &)));

                        connect(pTcpConnect,
                                SIGNAL(ClientReceive_CLIENT_RET(const QString &, const QJsonObject &)),
                                this, SLOT(receive_CLIENT_RET(const QString &, const QJsonObject &)));

                        connect(pTcpConnect,
                                SIGNAL(ClientReceive_SERVER_RET(const QJsonObject &)),
                                this, SLOT(receive_SERVER_RET(const QJsonObject &)));

                        udpSocket->deleteLater();
                        udpSocket = nullptr;
                        Connected = true;
                        waitCondition.wakeAll();
                        waitMutex.unlock();
                        return;
                    } else {
                        logger.error("Tcp Connect Time Out");
                        throw std::runtime_error("Tcp Connect Time Out");
                    }
                }
            }
        }
    } else {
        logger.error("Json error: {}", err.errorString());
    }
}

void RCS_Client::receive_GET(const QString &from, const QString &var, const QJsonObject &info) {
    auto it = callBackMap.find(var);
    if (it != callBackMap.end() && (*it).first) {
        pTcpConnect->send_PUSH(from, var, ((*it).first)(from, info));
        logger.info("Receives a GET request from '{}', gets the '{}' variable", from, var);
    } else {
        pTcpConnect->send_CLIENT_RET(from, {{"error", "variable is not registered"},
                                            {"var",   var}});
        logger.error("GET request from '{}', the requested '{}' variable is not registered", from,
                     var);
    }
}

void RCS_Client::receive_BROADCAST(const QString &from, const QString &broadcastName, const QJsonObject &val) {
    logger.info("Receives a BROADCAST from '{}', broadcastName:'{}'", from, broadcastName);
    emit BROADCAST(from, broadcastName, val);
}

void RCS_Client::receive_PUSH(const QString &from, const QString &var, const QJsonObject &val) {
    auto it = callBackMap.find(var);
    if (it != callBackMap.end()) {
        pTcpConnect->send_CLIENT_RET(from, {{"error", "variable is not registered"},
                                            {"var",   var}});
        logger.error("PUSH request from '{}', the requested '{}' variable is not registered", from, var);
    } else if ((*it).second) {
        ((*it).second)(from, val);
        logger.info("Receives a PUSH request from '{}', push the '{}' variable", from, var);
    } else {
        pTcpConnect->send_CLIENT_RET(from, {{"error", "variable is read only"},
                                            {"var",   var}});
        logger.error("PUSH request from '{}', the requested '{}' variable is read only", from, var);
    }
}

void RCS_Client::receive_SERVER_RET(const QJsonObject &ret) {
    logger.warn("Service return {}", ret);
    emit RETURN(TcpConnect::SERVER_RET, ret);
}

void RCS_Client::receive_CLIENT_RET(const QString &from, const QJsonObject &ret) {
    logger.warn("Client return {}", ret);
    emit RETURN(TcpConnect::CLIENT_RET, ret);
}

void RCS_Client::RegisterCallBack(const QString &name, const getCallback &getter, const setCallback &setter) {
    callBackMap.insert(name, {getter, setter});
}

void RCS_Client::RegisterCallBack(const QString &name, const getCallback &getter) {
    callBackMap.insert(name, {getter, nullptr});
}

int RCS_Client::UnregisterCallBack(const QString &name) {
    return callBackMap.remove(name);
}
