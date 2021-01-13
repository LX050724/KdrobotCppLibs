/**
 * @file RCS_Server.cpp
 * @author yao
 * @date 2021年1月13日
 */

#include "RCS_Server.h"
#include <QTcpSocket>
#include <QJsonObject>
#include <QFuture>

void RCS_Server::tcpServer_newConnection() {
    while (pTcpServer->hasPendingConnections()) {
        QTcpSocket *socket = pTcpServer->nextPendingConnection();
        connect(new TcpConnect(socket), SIGNAL(ServerReceive_HEAD(TcpConnect * , QString)),
                this, SLOT(TcpConnect_receive_HEAD(TcpConnect * , QString)));
    }
}

void RCS_Server::TcpConnect_disconnected(QString name) {
    logger.warn("client '{}' disconnected", name.toStdString());
    QMutexLocker lk(&mutex);
    clientList.remove(name);
}

void RCS_Server::TcpConnect_receive_HEAD(TcpConnect *pTcpConnect, QString name) {
    QMutexLocker lk(&mutex);
    clientList.insert(name, pTcpConnect);
    logger.info("client '{}' Connected", name.toStdString());
    connect(pTcpConnect, SIGNAL(Receive_BROADCAST(QString, QString, QJsonObject)),
            this, SLOT(TcpConnect_receive_BROADCAST(QString, QString, QJsonObject)));

    connect(pTcpConnect, SIGNAL(ServerReceive_GET(QString, QString, QString)),
            this, SLOT(TcpConnect_receive_GET(QString, QString, QString)));

    connect(pTcpConnect, SIGNAL(ServerReceive_PUSH(QString, QString, QString, QJsonObject)),
            this, SLOT(TcpConnect_receive_PUSH(QString, QString, QString, QJsonObject)));

    connect(pTcpConnect, SIGNAL(ServerReceive_CLIENT_RET(QString, QString, QJsonObject)),
            this, SLOT(TcpConnect_receive_CLIENT_RET(QString, QString, QJsonObject)));

    connect(pTcpConnect, SIGNAL(disconnected(QString)),
            this, SLOT(TcpConnect_disconnected(QString)));
}

void RCS_Server::TcpConnect_receive_BROADCAST(QString from, QString broadcastName, QJsonObject message) {
    for (const auto &client : clientList) {
        if (client->name != from)
            client->send_BROADCAST(from, broadcastName, message);
    }
    logger.info("broadcast '{}' from '{}'", broadcastName.toStdString(), from.toStdString());
}

void RCS_Server::TcpConnect_receive_PUSH(QString from, QString sendTo, QString var, QJsonObject obj) {
    auto it = clientList.find(sendTo);
    if (it != clientList.end()) {
        it.value()->send_PUSH(sendTo, var, obj);
        logger.info("forwarding PUSH request from '{}' to '{}'", from.toStdString(), sendTo.toStdString());
    } else {
        logger.error("not find client '{}'", sendTo.toStdString());
        clientList.find(from).value()->send_SERVER_RET({{"error", "not find client '" + sendTo + '\''}});
    }
}

void RCS_Server::TcpConnect_receive_GET(QString from, QString sendTo, QString var) {
    auto it = clientList.find(sendTo);
    if (it != clientList.end()) {
        it.value()->send_GET(from, var);
        logger.info("forwarding GET request from '{}' to '{}'", from.toStdString(), sendTo.toStdString());
    } else {
        logger.error("not find client '{}'", sendTo.toStdString());
        clientList.find(from).value()->send_SERVER_RET({{"error", "not find client '" + sendTo + '\''}});
    }
}

void RCS_Server::TcpConnect_receive_CLIENT_RET(QString from, QString sendTo, QJsonObject ret) {
    auto it = clientList.find(sendTo);
    if (it != clientList.end()) {
        it.value()->send_CLIENT_RET(from, ret);
        logger.info("forwarding CLIENT_RET request sendTo '{}' to '{}'", sendTo.toStdString(), sendTo.toStdString());
    } else {
        logger.error("not find client '{}'", sendTo.toStdString());
        clientList.find(sendTo).value()->send_SERVER_RET({{"error", "not find client '" + sendTo + '\''}});
    }
}