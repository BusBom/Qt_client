#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QString>
#include <QSqlDatabase>

class DBManager {
public:
    static DBManager& instance();

    bool connectToDatabase();  // 연결 시도
    bool addUser(const QString& email, const QString& password);  // 회원 추가
    bool isValidUser(const QString& email, const QString& password);  // 로그인 체크

private:
    QSqlDatabase db;
    DBManager();

    DBManager(const DBManager&) = delete;
    DBManager& operator=(const DBManager&) = delete;

    void init();
};

#endif // DBMANAGER_H

