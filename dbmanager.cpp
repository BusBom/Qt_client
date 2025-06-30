#include "dbmanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

DBManager::DBManager(QObject *parent) : QObject(parent) {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("user.db");

    if (!db.open()) {
        qDebug() << "데이터베이스 연결 실패:" << db.lastError().text();
    } else {
        qDebug() << "데이터베이스 연결 성공!";
        init();
    }
}

void DBManager::init() {
    QSqlQuery query;
    QString createTable = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            email TEXT UNIQUE,
            password TEXT
        )
    )";
    if (!query.exec(createTable)) {
        qDebug() << "테이블 생성 실패:" << query.lastError().text();
    }
}

bool DBManager::addUser(const QString &email, const QString &password) {
    QSqlQuery query;
    query.prepare("INSERT INTO users (email, password) VALUES (?, ?)");
    query.addBindValue(email);
    query.addBindValue(password);

    if (!query.exec()) {
        qDebug() << "회원가입 실패:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DBManager::isUserValid(const QString &email, const QString &password) {
    QSqlQuery query;
    query.prepare("SELECT * FROM users WHERE email = ? AND password = ?");
    query.addBindValue(email);
    query.addBindValue(password);

    if (query.exec() && query.next()) {
        return true;
    }
    return false;
}
