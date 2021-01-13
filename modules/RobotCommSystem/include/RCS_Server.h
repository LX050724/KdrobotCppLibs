#ifndef KDROBOTCPPLIBS_ROBOTCOMMSYSTEMSERVER_H
#define KDROBOTCPPLIBS_ROBOTCOMMSYSTEMSERVER_H

#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <QTcpServer>
#include "spdlogger.h"
#include "HostAddressRadio.h"
#include "TcpConnect.h"

class RCS_Server : public QObject {
Q_OBJECT

    spdlogger logger;
    HostAddressRadio *hostAddressRadio;
    QTcpServer *pTcpServer;
    QMutex mutex;
    QMap<QString, TcpConnect *> clientList;
public:
    RCS_Server() : logger(__FUNCTION__) {
        hostAddressRadio = new HostAddressRadio(this);
        pTcpServer = new QTcpServer(this);
        pTcpServer->listen(QHostAddress::Any, 8850);
        connect(pTcpServer, SIGNAL(newConnection()), this, SLOT(tcpServer_newConnection()));
    }

protected slots:

    void tcpServer_newConnection();

    void TcpConnect_receive_HEAD(TcpConnect *pTcpConnect, QString name);

    void TcpConnect_receive_BROADCAST(QString from, QString broadcastName, QJsonObject message);

    void TcpConnect_receive_PUSH(QString from, QString sendTo, QString var, QJsonObject obj);

    void TcpConnect_receive_GET(QString from, QString sendTo, QString var);

    void TcpConnect_receive_CLIENT_RET(QString from, QString sendTo, QJsonObject ret);

    void TcpConnect_disconnected(QString name);
};


#endif
