#include "JsonConfig.h"
#include <QJsonObject>
#include <QFile>

JsonConfig::JsonConfig(const QString &filename) : logger(__FUNCTION__) {
    opened = open(filename);
}

bool JsonConfig::open(const QString &filename) {
    QMutexLocker lk(&mut);
    QFile file(filename);
    if (!file.exists()) {
        return false;
    }
    file.open(QIODevice::ReadOnly);
    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll(), &jsonError);
    file.close();

    if (!jsonDoc.isNull() && (jsonError.error == QJsonParseError::NoError)) {
        jsonDocument = jsonDoc;
        Path = filename;
        return true;
    } else return false;
}

QJsonValue JsonConfig::findObject(const QString &path) {
    QMutexLocker lk(&mut);
    QStringList list = path.split('/');
    auto object = jsonDocument.object();
    for (const QString &node : list) {
        auto TmpObj = object[node];
        if(!TmpObj.isNull() && TmpObj.isObject()) {
            object = TmpObj.toObject();
        } else return TmpObj;
    }
    return object;
}

bool JsonConfig::isOpen() {
    return opened;
}

const QString &JsonConfig::getPath() const {
    return Path;
}
