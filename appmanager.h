#ifndef APPMANAGER_H
#define APPMANAGER_H

#include <QStackedWidget>

class LoginPage;
class SignUpPage;
class MainWindow;

class AppManager : public QStackedWidget {
    Q_OBJECT

public:
    AppManager(QWidget *parent = nullptr);

private:
    LoginPage *loginPage;
    SignUpPage *signUpPage;
    MainWindow *mainWindow;

private slots:
    void showSignupPage();    // to signup page
    void showLoginPage();     // to login page
    void loginSuccess();      // log in -> main dashboard
};

#endif // APPMANAGER_H
