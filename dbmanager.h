// dbmanager.h
#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QString>
#include <QSqlDatabase>

class DBManager {
public:
    static DBManager& instance();

    bool connectToDatabase();
    bool addUser(const QString& email, const QString& password);
    bool isValidUser(const QString& email, const QString& password);

private:
    QSqlDatabase db;
    DBManager();
};

#endif // DBMANAGER_H
