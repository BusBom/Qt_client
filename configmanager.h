#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QString>
#include <QJsonObject>

class ConfigManager {
public:
    static void load();
    static QString getValue(const QString &key);

private:
    static QJsonObject config;
};

#endif // CONFIGMANAGER_H
