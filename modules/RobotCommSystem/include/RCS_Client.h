#ifndef KDROBOTCPPLIBS_RCS_CLIENT_H
#define KDROBOTCPPLIBS_RCS_CLIENT_H

#include <functional>
#include <QtCore>
#include <QObject>
#include <QUdpSocket>
#include <QNetworkAddressEntry>
#include <QWaitCondition>
#include "TcpConnect.h"

class RCS_Client : QObject {
Q_OBJECT
    spdlogger logger;
    QList<QNetworkAddressEntry> HostAddressEntry;
    QUdpSocket *udpSocket = nullptr;
    TcpConnect *pTcpConnect = nullptr;

    QMutex udpMutex;
    QMutex waitMutex;
    QWaitCondition waitCondition;
    bool Connected = false;
    QString ClientName;

    QMap<QString, std::pair<std::function<QJsonObject(const QString &)>,
            std::function<void(const QString &, const QJsonObject &)>>> GetCallBackMap;
public:

    /**
     * 构造函数
     * @param _ClientName 客户端名
     * @param parent 父对象
     */
    RCS_Client(const QString &_ClientName, QObject *parent = nullptr);

    /**
     * 析构函数
     */
    ~RCS_Client() {
        if (pTcpConnect != nullptr)
            pTcpConnect->deleteLater();
    }

    /**
     * 注册GET请求和PUSH请求回调
     * @param name 注册变量名
     * @param getter getter方法 第一参数为GET请求来源，返回值为相应参数的值Json格式
     *               简单写法为<code>return {{"KeyString", val}, {"KeyString", val}, ...}};</code>
     * @param setter setter方法 第一参数为PUSH请求来源，第二参数为PUSH值
     */
    void RegisterCallBack(const QString &name, const std::function<QJsonObject(const QString &)> &getter,
                          const std::function<void(const QString &, const QJsonObject &)> &setter) {
        GetCallBackMap.insert(name, {getter, setter});
    }

    /**
     * 判断链接就绪
     * @return 链接就绪
     */
    inline bool isConnected() {
        return Connected;
    }

    /**
     * 等待链接就绪
     * @param deadline 超时时间
     * @return 链接成功
     */
    bool waitConnected(QDeadlineTimer deadline = QDeadlineTimer(QDeadlineTimer::Forever)) {
        return Connected || waitCondition.wait(&waitMutex, deadline);
    }

    /**
     * 发送广播
     * @param bordcastName 广播名
     * @param val 广播内容
     */
    inline void BROADCAST(const QString &bordcastName, const QJsonObject &val) {
        if (waitConnected()) pTcpConnect->send_BROADCAST(bordcastName, val);
    }

    /**
     * 发送GET请求
     * @param target 请求目标客户端
     * @param var 变量名
     */
    inline void GET(const QString &target, const QString &var) {
        if (waitConnected()) pTcpConnect->send_GET(target, var);
    }

    /**
     * 发送PUSH请求
     * @param target 请求目标客户端
     * @param var 变量名
     * @param val 变量值
     */
    inline void PUSH(const QString &target, const QString &var, const QJsonObject &val) {
        if (waitConnected()) pTcpConnect->send_PUSH(target, var, val);
    }

    /* 内部槽用户无需关心 */
protected slots:

    void UdpReadyRead();

    void receive_GET(QString from, QString var);

    void receive_PUSH(QString from, QString var, QJsonObject val);

    void receive_BROADCAST(QString from, QString broadcastName, QJsonObject val);

    void receive_SERVER_RET(QJsonObject ret);

    void receive_CLIENT_RET(QString from, QJsonObject ret);

signals:

    /**
     * 返回值信号量
     * @param type
     * @param ret
     */
    void RETURN(TcpConnect::PACK_TYPE type, const QJsonObject &ret);

    /**
     * 收到广播信号量
     * @param from 来源
     * @param broadcastName 广播名
     * @param message 广播消息
     */
    void BROADCAST(const QString &from, const QString &broadcastName, const QJsonObject &message);
};


#endif
