/**
 * @file QtCompatibilityLayer.h
 * @author yao
 * @date 2021年01月13日
 * @brief Qt类型兼容层，以后有需要再拓展
 */

#ifndef KDROBOTCPPLIBS_QTCOMPATIBILITYLAYER_H
#define KDROBOTCPPLIBS_QTCOMPATIBILITYLAYER_H

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/fmt/bin_to_hex.h"

#include <QtCore>
#include <QtNetwork/QHostAddress>

template<typename OStream>
OStream &operator<<(OStream &os, const QString &c) {
    return os << c.toLocal8Bit().toStdString();
}

template<typename OStream>
OStream &operator<<(OStream &os, const QByteArray &c) {
    return os << c.toStdString();
}

template<typename OStream>
OStream &operator<<(OStream &os, const QJsonObject &c) {
    return os << QJsonDocument(c);
}

template<typename OStream>
OStream &operator<<(OStream &os, const QJsonDocument &c) {
    return os << c.toJson();
}

template<typename OStream>
OStream &operator<<(OStream &os, const QHostAddress &c) {
    return os << c.toString();
}

#endif
