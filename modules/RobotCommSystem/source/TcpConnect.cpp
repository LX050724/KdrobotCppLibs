#include <QJsonObject>
#include "TcpConnect.h"

TcpConnect::TcpConnect(QTcpSocket *Socket, const QString &_name) : name(_name), logger(__FUNCTION__) {
    Socket->setParent(this);
    if (name.isEmpty()) {
        timer = new QTimer(this);
        timer->callOnTimeout(this, &::TcpConnect::WaitHEADTimeout);
        timer->start(10e3);
    }
    qThread = new QThread;
    moveToThread(qThread);
    qThread->start();
    this->socket = Socket;
    connect(Socket, SIGNAL(readyRead()), this, SLOT(Socket_readyRead()));
    connect(this, SIGNAL(Thread_write(QByteArray)), this, SLOT(write(QByteArray)),
            Qt::QueuedConnection);
    connect(Socket, &QTcpSocket::disconnected, this, [=]() {
        logger.warn("{}: disconnected", name);
        emit disconnected(name);
        deleteLater();
    });

    /* 构造没有连接名为服务端链接 */
    if (!name.isEmpty()) {
        send_HEAD();
        mode = CLIENT;
    } else {
        mode = SERVER;
    }
}

void TcpConnect::Socket_readyRead() {
    ReceiveBuff.push_back(socket->readAll());
    restart:
    auto dataPtr = ReceiveBuff.constData();
    int size = ReceiveBuff.size();
    for (int i = 0; i < size; i++) {
        if (dataPtr[i] == (char) 0x5a) {
            for (int j = i + 1; j < size; j++) {
                if (dataPtr[j] == (char) 0xa5) {
                    QByteArray MD5(dataPtr + j + 1, 16);
                    QByteArray Data(dataPtr + i + 1, j - i - 1);
                    QByteArray MD5Check = QCryptographicHash::hash(Data, QCryptographicHash::Md5);
                    if (MD5 == MD5Check) {
                        DecodeJson(Data);
                        ReceiveBuff.remove(0, j + 17);
                        goto restart;
                    }
                }
            }
        }
    }
}

void TcpConnect::DecodeJson(QByteArray &data) {
    QJsonParseError error;
    QJsonDocument jdom = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError) {
        logger.error("Json error:{} {}\n{}", error.error, error.errorString(), data);
        return;
    }
    QJsonObject jobj = jdom.object();
    PACK_TYPE type = (PACK_TYPE) jobj.value("type").toInt(-1);
    QJsonObject body = jobj.value("body").toObject();
    logger.debug("receive, Address={}, type={}, size={}", socket->peerAddress(), PACK_TYPE_ToString(type), data.size());

    switch (type) {
        case HEAD: {
            QString string = jobj.value("name").toString();
            if (string.isEmpty()) {
                logger.error("not find Address\n{}", data);
                return;
            }
            if (name.isEmpty()) {
                name = string;
                setObjectName(string);
                socket->setObjectName(string + "_Socket");
                timer->stop();
                logger.debug("Receive HEAD name={}", name);
                switch (mode) {
                    case SERVER:
                        emit ServerReceive_HEAD(this, string);
                        break;
                    case CLIENT:
                        break;
                }
            } else logger.error("Repeat set name \"{}\" to \"{}\"", name, string);
            break;
        }
        case BROADCAST: {
            emit Receive_BROADCAST(name, jobj.find("bordcastName")->toString(),
                                   jobj.find("bordcast")->toObject());
            break;
        }
        case PUSH: {
            QString from_sendTo = jobj.find("from_sendTo")->toString();
            QString tar_var = jobj.find("var")->toString();
            QJsonObject tar_val = jobj.find("val")->toObject();
            switch (mode) {
                case SERVER:
                    emit ServerReceive_PUSH(name, from_sendTo, tar_var, tar_val);
                    break;
                case CLIENT:
                    emit ClientReceive_PUSH(from_sendTo, tar_var, tar_val);
                    break;
            }
            break;
        }
        case GET: {
            QString from_sendTo = jobj.find("from_sendTo")->toString();
            QString tar_var = jobj.find("var")->toString();
            switch (mode) {
                case SERVER:
                    emit ServerReceive_GET(name, from_sendTo, tar_var);
                    break;
                case CLIENT:
                    emit ClientReceive_GET(from_sendTo, tar_var);
                    break;
            }
            break;
        }
        case SERVER_RET: {
            emit ClientReceive_SERVER_RET(jobj.find("ret")->toObject());
            break;
        }
        case CLIENT_RET: {
            QString from_sendTo = jobj.find("from_sendTo")->toString();
            QJsonObject ret = jobj.find("ret")->toObject();
            switch (mode) {
                case SERVER:
                    emit ServerReceive_CLIENT_RET(name, from_sendTo, ret);
                    break;
                case CLIENT:
                    emit ClientReceive_CLIENT_RET(from_sendTo, ret);
                    break;
            }
            break;
        }
        default:
            logger.error("Unknown type {}\n{}", type, data);
    }
}

void TcpConnect::write(QByteArray data) {
    if (socket->thread() != QThread::currentThread()) {
        emit Thread_write(data);
        return;
    }
    QByteArray MD5 = QCryptographicHash::hash(data, QCryptographicHash::Md5);
    data.push_front((char) 0x5a);
    data.push_back((char) 0xa5);
    data.push_back(MD5);
    if (!socket->isWritable() && !socket->waitForBytesWritten(100)) {
        logger.error("{}: waitForBytesWritten time out!", name);
        return;
    }
    socket->write(data);
}

void TcpConnect::WaitHEADTimeout() {
    logger.error("Wait HEAD time out IP={}", socket->peerAddress().toString());
    if (qThread != nullptr) {
        qThread->quit();
        qThread->deleteLater();
        qThread = nullptr;
    }
    this->deleteLater();
}

TcpConnect::~TcpConnect() {
    if (qThread != nullptr) {
        qThread->quit();
        qThread = nullptr;
    }
}

void TcpConnect::send_HEAD() {
    QJsonObject jobj;
    jobj.insert("type", HEAD);
    jobj.insert("name", name);
    write(jobj);
}

void TcpConnect::send_BROADCAST(const QString &from, const QString &bordcastName, const QJsonObject &message) {
    QJsonObject jobj;
    jobj.insert("type", BROADCAST);
    jobj.insert("from", from);
    jobj.insert("bordcastName", bordcastName);
    jobj.insert("bordcast", message);
    write(jobj);
}

void TcpConnect::send_BROADCAST(const QString &bordcastName, const QJsonObject &message) {
    QJsonObject jobj;
    jobj.insert("type", BROADCAST);
    jobj.insert("bordcastName", bordcastName);
    jobj.insert("bordcast", message);
    write(jobj);
}

void TcpConnect::send_PUSH(const QString &from_sendTo, const QString &var, const QJsonObject &val) {
    QJsonObject jobj;
    jobj.insert("type", PUSH);
    jobj.insert("from_sendTo", from_sendTo);
    jobj.insert("var", var);
    jobj.insert("val", val);
    write(jobj);
}

void TcpConnect::send_GET(const QString &from_sendTo, const QString &var) {
    QJsonObject jobj;
    jobj.insert("type", GET);
    jobj.insert("from_sendTo", from_sendTo);
    jobj.insert("var", var);
    write(jobj);
}

void TcpConnect::send_SERVER_RET(const QJsonObject &ret) {
    QJsonObject jobj;
    jobj.insert("type", SERVER_RET);
    jobj.insert("ret", ret);
    write(jobj);
}

void TcpConnect::send_CLIENT_RET(const QString &from_sendTo, const QJsonObject &ret) {
    QJsonObject jobj;
    jobj.insert("type", CLIENT_RET);
    jobj.insert("from_sendTo", from_sendTo);
    jobj.insert("ret", ret);
    write(jobj);
}

const char *TcpConnect::PACK_TYPE_ToString(TcpConnect::PACK_TYPE type) {
    switch (type) {
        case HEAD:
            return "HEAD";
        case BROADCAST:
            return "BROADCAST";
        case PUSH:
            return "PUSH";
        case GET:
            return "GET";
        case SERVER_RET:
            return "SERVER_RET";
        case CLIENT_RET:
            return "CLIENT_RET";
    }
    return "Unknown";
}
