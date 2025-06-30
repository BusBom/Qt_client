#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class QLabel;

class LoginPage : public QWidget {
    Q_OBJECT

public:
    explicit LoginPage(QWidget *parent = nullptr);

signals:
    void loginSuccess();             // 로그인 성공 → 대시보드로
    void switchToSignupPage();       // 회원가입 창으로 전환

private slots:
    void handleLoginClicked();

private:
    QLineEdit *emailInput;
    QLineEdit *passwordInput;
    QPushButton *loginBtn;
    QLabel *signUpLink;
};

#endif // LOGINPAGE_H
