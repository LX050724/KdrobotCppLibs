#include <QNetworkInterface>
#include <QNetworkDatagram>
#include "RCS_Client.h"

RCS_Client::RCS_Client(const QString &_ClientName, QObject *parent) :
        QObject(parent), logger(__FUNCTION__), ClientName(_ClientName) {
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
    udpSocket->bind(8849, QUdpSocket::ShareAddress);
    connect(udpSocket, SIGNAL(readyRead()), this, SLOT(UdpReadyRead()));
    waitMutex.lock();
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
                    tcpSocket->connectToHost(remoteIP, 8850);
                    if (tcpSocket->waitForConnected(5000)) {
                        pTcpConnect = new TcpConnect(tcpSocket, ClientName);

                        connect(pTcpConnect, SIGNAL(ClientReceive_GET(QString, QString)),
                                this, SLOT(receive_GET(QString, QString)));

                        connect(pTcpConnect, SIGNAL(ClientReceive_PUSH(QString, QString, QJsonObject)),
                                this, SLOT(receive_PUSH(QString, QString, QJsonObject)));

                        connect(pTcpConnect, SIGNAL(Receive_BROADCAST(QString, QString, QJsonObject)),
                                this, SLOT(receive_BROADCAST(QString, QString, QJsonObject)));

                        connect(pTcpConnect, SIGNAL(ClientReceive_CLIENT_RET(QString, QJsonObject)),
                                this, SLOT(receive_CLIENT_RET(QString, QJsonObject)));

                        connect(pTcpConnect, SIGNAL(ClientReceive_SERVER_RET(QJsonObject)),
                                this, SLOT(receive_SERVER_RET(QJsonObject)));

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

void RCS_Client::receive_GET(QString from, QString var) {
    auto it = GetCallBackMap.find(var);
    if (it != GetCallBackMap.end() && (*it).first) {
        pTcpConnect->send_PUSH(from, var, ((*it).first)(from));
        logger.info("Receives a GET request from '{}', gets the '{}' variable", from, var);
    } else {
        pTcpConnect->send_CLIENT_RET(from, {{"error", "variable is not registered"},
                                            {"var",   var}});
        logger.error("GET request from '{}', the requested '{}' variable is not registered", from,
                     var);
    }
}

void RCS_Client::receive_BROADCAST(QString from, QString broadcastName, QJsonObject val) {
    logger.info("Receives a BROADCAST from '{}', broadcastName:'{}'", from, broadcastName);
    emit BROADCAST(from, broadcastName, val);
}

void RCS_Client::receive_PUSH(QString from, QString var, QJsonObject val) {
    auto it = GetCallBackMap.find(var);
    if (it != GetCallBackMap.end() && (*it).second) {
        ((*it).second)(from, val);
        logger.info("Receives a PUSH request from '{}', push the '{}' variable", from, var);
    } else {
        QJsonObject jobj;
        pTcpConnect->send_CLIENT_RET(from, {{"error", "variable is not registered"},
                                            {"var",   var}});
        logger.error("PUSH request from '{}', the requested '{}' variable is not registered", from,
                     var);
    }
}

void RCS_Client::receive_SERVER_RET(QJsonObject ret) {
    logger.warn("Service return {}", ret);
    emit RETURN(TcpConnect::SERVER_RET, ret);
}

void RCS_Client::receive_CLIENT_RET(QString from, QJsonObject ret) {
    logger.warn("Client return {}", ret);
    emit RETURN(TcpConnect::CLIENT_RET, ret);
}
