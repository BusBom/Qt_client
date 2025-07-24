#ifndef SIGNUPPAGE_H
#define SIGNUPPAGE_H

#include <QWidget>

class QLineEdit;
class QPushButton;
class QLabel;

class SignUpPage : public QWidget {
    Q_OBJECT

public:
    explicit SignUpPage(QWidget *parent = nullptr);

signals:
    void signupSuccess();         // 가입 완료 → 로그인 페이지로
    void switchToLoginPage();     // 이미 계정 있음 → 로그인으로

private slots:
    void handleSignUpClicked();

private:
    QLineEdit *emailInput;
    QLineEdit *passwordInput;
    QLineEdit *passwordConfirmInput;
    QPushButton *signUpBtn;
    QLabel *loginLink;
    QLabel *passwordMismatchLabel;
};

#endif // SIGNUPPAGE_H
