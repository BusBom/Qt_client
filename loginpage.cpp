#include "loginpage.h"
#include "dbmanager.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QFont>
#include <QCursor>

LoginPage::LoginPage(QWidget *parent) : QWidget(parent) {

    // 전체 배경
    //this->setStyleSheet("background-color: #1e1e1e;");
    QPalette pal = this->palette();
    pal.setColor(QPalette::Window, QColor("#1e1e1e"));  // 원하는 배경색
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    this->setFixedSize(1240, 650);

    // 로그인 창
    QFrame *whiteBox = new QFrame;
    whiteBox->setFixedSize(450, 350);
    whiteBox->setStyleSheet("background-color: white; border-radius: 20px;");

    QLabel *titleLabel = new QLabel("Log In");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: black;");
    titleLabel->setAlignment(Qt::AlignLeft);

    // Email Label + Input
    QLabel *emailLabel = new QLabel("Email Address");
    emailLabel->setStyleSheet("font-size: 12px; color: #555;");
    emailInput = new QLineEdit();
    emailInput->setPlaceholderText("이메일을 입력해주세요.");
    emailInput->setStyleSheet("background-color: #f5f7fa; padding: 10px; border: none;");

    // Password Label + Input
    QLabel *passwordLabel = new QLabel("Password");
    passwordLabel->setStyleSheet("font-size: 12px; color: #555;");
    passwordInput = new QLineEdit();
    passwordInput->setPlaceholderText("비밀번호를 입력해주세요.");
    passwordInput->setEchoMode(QLineEdit::Password);
    passwordInput->setStyleSheet("background-color: #f5f7fa; padding: 10px; border: none;");

    loginBtn = new QPushButton("LOG IN");
    loginBtn->setFixedHeight(40);
    loginBtn->setStyleSheet(R"(
    background-color: #f28b40;
    border: none;
    border-radius: 12px;
    color: white;
    font-weight: bold;
    font-size: 14px;
    )");

    connect(loginBtn, &QPushButton::clicked, this, &LoginPage::handleLoginClicked);

    QFrame *divider = new QFrame();
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("background-color: #ccc;");
    divider->setFixedHeight(1);

    signUpLink = new QLabel("<a href='#'>No account yet? Sign Up</a>");
    signUpLink->setStyleSheet("color: #3061e6;");
    signUpLink->setTextFormat(Qt::RichText);
    signUpLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
    signUpLink->setCursor(Qt::PointingHandCursor);
    connect(signUpLink, &QLabel::linkActivated, this, [=]() {
        emit switchToSignupPage();
    });

    // 내부 레이아웃 구성
    QVBoxLayout *formLayout = new QVBoxLayout;
    formLayout->setAlignment(Qt::AlignCenter);
    formLayout->addWidget(titleLabel);
    formLayout->addSpacing(10);
    formLayout->addWidget(emailLabel);
    formLayout->addWidget(emailInput);
    formLayout->addWidget(passwordLabel);
    formLayout->addWidget(passwordInput);
    formLayout->addSpacing(10);
    formLayout->addWidget(loginBtn);
    formLayout->addSpacing(15);
    formLayout->addWidget(divider);
    formLayout->addWidget(signUpLink);
    formLayout->setContentsMargins(30, 20, 30, 20);
    whiteBox->setLayout(formLayout);

    // 중앙 정렬 레이아웃
    QHBoxLayout *centerLayout = new QHBoxLayout;
    centerLayout->addStretch();
    centerLayout->addWidget(whiteBox);
    centerLayout->addStretch();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(centerLayout);
    mainLayout->setAlignment(centerLayout, Qt::AlignCenter);  // 수직 중앙 정렬

}


void LoginPage::handleLoginClicked() {
    QString email = emailInput->text().trimmed();
    QString password = passwordInput->text().trimmed();

    if (email.isEmpty() || password.isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("입력 오류");
        msgBox.setText("이메일과 비밀번호를 입력해주세요.");
        msgBox.setStyleSheet(R"(
            QMessageBox {
                background-color: white;
            }
            QLabel {
                background-color: white;
                color: black;
                font-size: 14px;
            }
            QPushButton {
                background-color: #f28b40;
                color: white;
                border-radius: 10px;
                padding: 5px 10px;
                font-weight: bold;
                min-width: 60px;
            }
        )");

        msgBox.exec();
        return;
    }

    if (DBManager::instance().isValidUser(email, password)) {
        emit loginSuccess();  // 로그인 성공 시 MainWindow로 이동
    } else {
        QMessageBox::critical(this, "로그인 실패", "이메일/비밀번호가 일치하지 않습니다.");
    }
}
