#ifndef KDROBOTCPPLIBS_JSONCONFIG_H
#define KDROBOTCPPLIBS_JSONCONFIG_H

#include <exception>

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMutex>
#include "spdlogger.h"

class JsonConfig {
    QJsonDocument jsonDocument;
    QString Path;
    bool opened = false;
    spdlogger logger;
    QMutex mut;
public:

    JsonConfig() : logger(__FUNCTION__) {};

    JsonConfig(const QString &filename);

    JsonConfig(const JsonConfig &copy) :
            logger(copy.logger), jsonDocument(copy.jsonDocument) {
        opened = copy.opened;
    }

    JsonConfig(JsonConfig &&m) :
            jsonDocument(std::move(m.jsonDocument)),
            Path(std::move(m.Path)), logger(std::move(m.logger)) {
        opened = m.opened;
    }


    JsonConfig operator=(const JsonConfig &jsonConfig) {
        this->logger = jsonConfig.logger;
        this->jsonDocument = jsonConfig.jsonDocument;
        this->opened = jsonConfig.opened;
        return *this;
    }

    bool open(const QString &filename = "config.json");

    bool isOpen();

    const QString &getPath() const;

    QJsonValue findObject(const QString &path);

    int findInt(const QString &path, int defaultValue, std::function<bool(int)> cmp = [](int a) { return true; }) {
        int val = findObject(path).toInt(defaultValue);
        if (!cmp(val)) {
            logger.error("'{}' missing or error", path.toStdString());
            throw std::runtime_error("config error");
        }
        return val;
    }

    inline int findInt(const QString &path, std::function<bool(int)> cmp = [](int a) { return true; }) {
        return findInt(path, 0, cmp);
    }

    static JsonConfig &factory(const QString &filename = "config.json") {
        static QMap<QString, JsonConfig> JsonConfigMap;
        if (!JsonConfigMap.contains(filename)) {
            return JsonConfigMap.insert(filename, JsonConfig(filename)).value();
        }
        return JsonConfigMap[filename];
    }
};


#endif //KDROBOTCPPLIBS_JSONCONFIG_H
