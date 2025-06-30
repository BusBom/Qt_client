#include "signuppage.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QFont>
#include <QCursor>
#include <QFrame>

SignUpPage::SignUpPage(QWidget *parent) : QWidget(parent) {
    // 배경 다크모드
    this->setStyleSheet("background-color: #1e1e1e;");
    this->setFixedSize(1280, 720);

    // 흰색 카드 박스
    QFrame *whiteBox = new QFrame;
    whiteBox->setFixedSize(450, 350);
    whiteBox->setStyleSheet("background-color: white; border-radius: 20px;");

    QLabel *titleLabel = new QLabel("Sign Up");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: black;");
    titleLabel->setAlignment(Qt::AlignLeft);

    QLabel *emailLabel = new QLabel("Email Address");
    emailLabel->setStyleSheet("font-size: 12px; color: #555;");
    emailInput = new QLineEdit();
    emailInput->setPlaceholderText("이메일을 입력해주세요.");
    emailInput->setStyleSheet("background-color: #f5f7fa; padding: 10px; border: none;");

    QLabel *passwordLabel = new QLabel("Password");
    passwordLabel->setStyleSheet("font-size: 12px; color: #555;");
    passwordInput = new QLineEdit();
    passwordInput->setPlaceholderText("비밀번호를 입력해주세요.");
    passwordInput->setEchoMode(QLineEdit::Password);
    passwordInput->setStyleSheet("background-color: #f5f7fa; padding: 10px; border: none;");

    signUpBtn = new QPushButton("SIGN UP");
    signUpBtn->setFixedHeight(40);
    signUpBtn->setStyleSheet(R"(
        background-color: #f28b40;
        border: none;
        border-radius: 20px;
        color: white;
        font-weight: bold;
        font-size: 14px;
    )");
    connect(signUpBtn, &QPushButton::clicked, this, &SignUpPage::handleSignUpClicked);

    // 🔻 회색 구분선
    QFrame *divider = new QFrame();
    divider->setFrameShape(QFrame::HLine);
    divider->setStyleSheet("background-color: #ccc;");
    divider->setFixedHeight(1);

    // 아래쪽 "Log In" 링크
    loginLink = new QLabel("<a href='#'>Log In</a>");
    loginLink->setStyleSheet("color: #3061e6;");
    loginLink->setTextFormat(Qt::RichText);
    loginLink->setTextInteractionFlags(Qt::TextBrowserInteraction);
    loginLink->setCursor(Qt::PointingHandCursor);
    connect(loginLink, &QLabel::linkActivated, this, [=]() {
        emit switchToLoginPage();
    });

    // 내부 레이아웃
    QVBoxLayout *formLayout = new QVBoxLayout;
    formLayout->addWidget(titleLabel);
    formLayout->addSpacing(10);
    formLayout->addWidget(emailLabel);
    formLayout->addWidget(emailInput);
    formLayout->addWidget(passwordLabel);
    formLayout->addWidget(passwordInput);
    formLayout->addSpacing(10);
    formLayout->addWidget(signUpBtn);
    formLayout->addSpacing(15);
    formLayout->addWidget(divider);
    formLayout->addWidget(loginLink);
    formLayout->setContentsMargins(30, 20, 30, 20);
    formLayout->setAlignment(Qt::AlignCenter);
    whiteBox->setLayout(formLayout);

    // 화면 중앙에 흰 박스 배치
    QHBoxLayout *centerLayout = new QHBoxLayout;
    centerLayout->addStretch();
    centerLayout->addWidget(whiteBox);
    centerLayout->addStretch();

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addStretch();
    mainLayout->addLayout(centerLayout);
    mainLayout->addStretch();
}

void SignUpPage::handleSignUpClicked() {
    QString email = emailInput->text().trimmed();
    QString password = passwordInput->text().trimmed();

    if (email.isEmpty() || password.isEmpty()) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("입력 오류");
        msgBox.setText("이메일과 비밀번호를 입력해주세요.");
        msgBox.setIcon(QMessageBox::Warning);

        msgBox.setStyleSheet(R"(
            QMessageBox {
                background-color: white;
            }
            QLabel {
                background-color: transparent;
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

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("가입 완료");
    msgBox.setText("가입완료");
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setStyleSheet(R"(
        QMessageBox {
            background-color: white;
        }
        QLabel {
            background-color: transparent;
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

    emit switchToLoginPage();
}
