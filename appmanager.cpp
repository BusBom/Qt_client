#include "appmanager.h"
#include "loginpage.h"
#include "signuppage.h"
#include "mainwindow.h"

AppManager::AppManager(QWidget *parent)
    : QStackedWidget(parent) {

    loginPage = new LoginPage;
    signUpPage = new SignUpPage;
    mainWindow = new MainWindow;

    addWidget(loginPage);   // index 0
    addWidget(signUpPage);  // index 1
    addWidget(mainWindow);  // index 2

    setCurrentWidget(loginPage);  // 첫 화면 = 로그인

    // ✅ 시그널 연결
    connect(loginPage, &LoginPage::switchToSignupPage, this, &AppManager::showSignupPage);
    connect(loginPage, &LoginPage::loginSuccess, this, &AppManager::loginSuccess);
    connect(signUpPage, &SignUpPage::switchToLoginPage, this, &AppManager::showLoginPage);
    connect(signUpPage, &SignUpPage::signupSuccess, this, &AppManager::showLoginPage);
}

void AppManager::showSignupPage() {
    setCurrentWidget(signUpPage);
}

void AppManager::showLoginPage() {
    setCurrentWidget(loginPage);
}

void AppManager::loginSuccess() {
    setCurrentWidget(mainWindow);
}
