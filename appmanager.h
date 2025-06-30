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
    void showSignupPage();    // 회원가입 페이지로
    void showLoginPage();     // 로그인 페이지로
    void loginSuccess();      // 로그인 성공 → 메인 대시보드
};

#endif // APPMANAGER_H
