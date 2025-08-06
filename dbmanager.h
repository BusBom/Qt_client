#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QString>
#include <QSqlDatabase>

class DBManager {
public:
    static DBManager& instance();

    bool connectToDatabase();
    bool addUser(const QString& email, const QString& password);  // add user
    bool isValidUser(const QString& email, const QString& password);  // check log in

private:
    QSqlDatabase db;
    DBManager();

    DBManager(const DBManager&) = delete;
    DBManager& operator=(const DBManager&) = delete;

    void init();
};

#endif // DBMANAGER_H

