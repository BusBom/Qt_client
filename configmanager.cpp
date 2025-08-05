#include "configmanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QDebug>
#include <QCoreApplication>

QJsonObject ConfigManager::config;

void ConfigManager::load() {
    QString path = QCoreApplication::applicationDirPath() + "/config.json";
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "❌ config.json 파일 열기 실패";
        return;
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        qWarning() << "❌ config.json 파싱 실패";
        return;
    }

    config = doc.object();
    qDebug() << "✅ config.json 로드 완료:" << config;
}

QString ConfigManager::getValue(const QString &key) {
    return config.value(key).toString();
}
