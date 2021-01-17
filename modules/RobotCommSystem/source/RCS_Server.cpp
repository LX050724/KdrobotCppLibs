/**
 * @file RCS_Server.cpp
 * @author yao
 * @date 2021年1月13日
 */

#include "RCS_Server.h"
#include <QTcpSocket>

void RCS_Server::tcpServer_newConnection() {
    while (pTcpServer->hasPendingConnections()) {
        QTcpSocket *socket = pTcpServer->nextPendingConnection();
        connect(new TcpConnect(socket), SIGNAL(ServerReceive_HEAD(TcpConnect * , const QString &)),
                this, SLOT(TcpConnect_receive_HEAD(TcpConnect * , const QString &)));
    }
}

void RCS_Server::TcpConnect_disconnected(const QString &name) {
    logger.warn("client '{}' disconnected", name);
    QMutexLocker lk(&mutex);
    clientList.remove(name);
}

void RCS_Server::TcpConnect_receive_HEAD(TcpConnect *pTcpConnect, const QString &name) {
    QMutexLocker lk(&mutex);
    if (clientList.contains(name)) {
        pTcpConnect->send_SERVER_RET({{"error",      "There is already a client with the same name"},
                                      {"disconnect", true}});
        return;
    }

    clientList.insert(name, pTcpConnect);
    logger.info("client '{}' Connected", name);
    connect(pTcpConnect, SIGNAL(Receive_BROADCAST(const QString &, const QString &, const QJsonObject &)),
            this, SLOT(TcpConnect_receive_BROADCAST(const QString &, const QString &, const QJsonObject &)));

    connect(pTcpConnect,
            SIGNAL(ServerReceive_GET(const QString &, const QString &, const QString &, const QJsonObject &)),
            this, SLOT(TcpConnect_receive_GET(const QString &, const QString &, const QString &, const QJsonObject &)));

    connect(pTcpConnect,
            SIGNAL(ServerReceive_PUSH(const QString &, const QString &, const QString &, const QJsonObject &)),
            this,
            SLOT(TcpConnect_receive_PUSH(const QString &, const QString &, const QString &, const QJsonObject &)));

    connect(pTcpConnect, SIGNAL(ServerReceive_CLIENT_RET(const QString &, const QString &, const QJsonObject &)),
            this, SLOT(TcpConnect_receive_CLIENT_RET(const QString &, const QString &, const QJsonObject &)));

    connect(pTcpConnect, SIGNAL(disconnected(const QString &)),
            this, SLOT(TcpConnect_disconnected(const QString &)));
    emit NewClient(pTcpConnect->socket->peerAddress(), name);
}

void RCS_Server::TcpConnect_receive_BROADCAST(const QString &from, const QString &broadcastName,
                                              const QJsonObject &message) {
    for (const auto &client : clientList) {
        if (client->name != from)
            client->send_BROADCAST(from, broadcastName, message);
    }
    logger.info("broadcast '{}' from '{}'", broadcastName, from);
}

void RCS_Server::TcpConnect_receive_PUSH(const QString &from, const QString &sendTo, const QString &var,
                                         const QJsonObject &obj) {
    auto it = clientList.find(sendTo);
    if (it != clientList.end()) {
        it.value()->send_PUSH(sendTo, var, obj);
        logger.info("forwarding PUSH request from '{}' to '{}'", from, sendTo);
    } else {
        logger.error("not find client '{}'", sendTo);
        clientList.find(from).value()->send_SERVER_RET({{"error", "not find client '" + sendTo + '\''}});
    }
}

void RCS_Server::TcpConnect_receive_GET(const QString &from, const QString &sendTo, const QString &var,
                                        const QJsonObject &info) {
    auto it = clientList.find(sendTo);
    if (it != clientList.end()) {
        it.value()->send_GET(from, var, info);
        logger.info("forwarding GET request from '{}' to '{}'", from, sendTo);
    } else {
        logger.error("not find client '{}'", sendTo);
        clientList.find(from).value()->send_SERVER_RET({{"error", "not find client '" + sendTo + '\''}});
    }
}

void RCS_Server::TcpConnect_receive_CLIENT_RET(const QString &from, const QString &sendTo, const QJsonObject &ret) {
    auto it = clientList.find(sendTo);
    if (it != clientList.end()) {
        it.value()->send_CLIENT_RET(from, ret);
        logger.info("forwarding CLIENT_RET request from '{}' to '{}'", from, sendTo);
    } else {
        logger.error("not find client '{}'", sendTo);
        clientList.find(sendTo).value()->send_SERVER_RET({{"error", "not find client '" + sendTo + '\''}});
    }
}

RCS_Server::RCS_Server(uint16_t TcpPort, bool udpRadio) : logger(__FUNCTION__) {
    if (udpRadio) hostAddressRadio = new HostAddressRadio(this);
    pTcpServer = new QTcpServer(this);
    if (!pTcpServer->listen(QHostAddress::Any, TcpPort)) {
        logger.error("TCP can't listing");
        throw std::runtime_error("TCP can't listing");
    }
    connect(pTcpServer, SIGNAL(newConnection()), this, SLOT(tcpServer_newConnection()));
}

QList<QString> RCS_Server::getClientNameList() {
    return clientList.keys();
}

size_t RCS_Server::getClientCount() {
    return clientList.count();
}

bool RCS_Server::disconnect(const QString &name) {
    auto it = clientList.find(name);
    if (it != clientList.end()) {
        (*it)->deleteLater();
        clientList.erase(it);
        return true;
    } else return false;
}
