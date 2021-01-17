/**
 * @file RCS_Server.h
 * @author yao
 * @date 2021年1月13日
 */

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
    HostAddressRadio *hostAddressRadio = nullptr;
    QTcpServer *pTcpServer;
    QMutex mutex;
    QMap<QString, TcpConnect *> clientList;
public:

    /**
     * 构造函数
     * @param TcpPort 使用的Tcp端口
     * @param udpRadio 是否开启Udp广播服务器IP
     */
    RCS_Server(uint16_t TcpPort = 8850, bool udpRadio = true);

    /**
     * 获取客户端名字列表
     * @return
     */
    QList<QString> getClientNameList();

    /**
     * 获取客户端数量
     * @return 客户端数量
     */
    size_t getClientCount();

    /**
     * 断开指定客户端连接
     * @param name 客户端名
     * @return 查询到客户端并断开返回true，客户端不在列表中返回false
     */
    bool disconnect(const QString &name);

signals:
    void NewClient(const QHostAddress &addr, const QString &name);

protected slots:

    void tcpServer_newConnection();

    void TcpConnect_receive_HEAD(TcpConnect *pTcpConnect, const QString &name);

    void TcpConnect_receive_BROADCAST(const QString &from, const QString &broadcastName, const QJsonObject &message);

    void TcpConnect_receive_PUSH(const QString &from, const QString &sendTo,
                                 const QString &var, const QJsonObject &obj);

    void TcpConnect_receive_GET(const QString &from, const QString &sendTo,
                                const QString &var, const QJsonObject &info);

    void TcpConnect_receive_CLIENT_RET(const QString &from, const QString &sendTo, const QJsonObject &ret);

    void TcpConnect_disconnected(const QString &name);
};


#endif
