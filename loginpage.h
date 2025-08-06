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
    void loginSuccess();
    void switchToSignupPage();

private slots:
    void handleLoginClicked();

private:
    QLineEdit *emailInput;
    QLineEdit *passwordInput;
    QPushButton *loginBtn;
    QLabel *signUpLink;
};

#endif // LOGINPAGE_H
