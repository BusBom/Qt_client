#include "signuppage.h"
#include "dbmanager.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QFont>
#include <QCursor>
#include <QFrame>
#include <QHBoxLayout>

SignUpPage::SignUpPage(QWidget *parent) : QWidget(parent) {
    // 배경 다크모드
    QPalette pal = this->palette();
    pal.setColor(QPalette::Window, QColor("#1e1e1e"));  // 원하는 배경색
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    this->setFixedSize(1240, 650);

    // 로그인 창
    QFrame *whiteBox = new QFrame;
    whiteBox->setFixedSize(450, 350);
    whiteBox->setStyleSheet("background-color: white; border-radius: 20px;");

    QLabel *titleLabel = new QLabel("Sign Up");
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: black;");
    titleLabel->setAlignment(Qt::AlignLeft);

    QLabel *emailLabel = new QLabel("Email Address");
    emailLabel->setStyleSheet("font-size: 11px; color: #555;");
    emailInput = new QLineEdit();
    emailInput->setPlaceholderText("이메일을 입력해주세요.");
    emailInput->setStyleSheet("background-color: #f5f7fa; padding: 10px; border: none; border-radius: 0px; ");
    emailInput->setFixedHeight(40);

    QLabel *passwordLabel = new QLabel("Password");
    passwordLabel->setStyleSheet("font-size: 11px; color: #555;");
    passwordInput = new QLineEdit();
    passwordInput->setPlaceholderText("비밀번호를 입력해주세요.");
    passwordInput->setEchoMode(QLineEdit::Password);
    passwordInput->setStyleSheet("background-color: #f5f7fa; padding: 10px; border: none; border-radius: 0px; ");
    passwordInput->setFixedHeight(40);

    QLabel *passwordConfrimLabel = new QLabel("Rewrite Password");
    passwordConfrimLabel->setStyleSheet("font-size: 11px; color: #555;");

    passwordMismatchLabel = new QLabel("※ 비밀번호가 일치하지 않습니다.");
    passwordMismatchLabel->setStyleSheet("color: red; font-size: 9px;");
    passwordMismatchLabel->hide();

    QHBoxLayout *confirmLabelLayout = new QHBoxLayout;
    confirmLabelLayout->addWidget(passwordConfrimLabel);
    confirmLabelLayout->addSpacing(10);
    confirmLabelLayout->addWidget(passwordMismatchLabel);
    confirmLabelLayout->addStretch();

    passwordConfirmInput = new QLineEdit();
    passwordConfirmInput->setPlaceholderText("비밀번호를 다시 입력해주세요.");
    passwordConfirmInput->setEchoMode(QLineEdit::Password);
    passwordConfirmInput->setStyleSheet("background-color: #f5f7fa; padding: 10px; border: none; border-radius: 0px;");
    passwordConfirmInput->setFixedHeight(40);

    signUpBtn = new QPushButton("SIGN UP");
    signUpBtn->setFixedHeight(40);
    signUpBtn->setStyleSheet(R"(
        background-color: #f28b40;
        border: none;
        border-radius: 12px;
        color: white;
        font-weight: bold;
        font-size: 14px;
    )");
    connect(signUpBtn, &QPushButton::clicked, this, &SignUpPage::handleSignUpClicked);

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
    formLayout->addLayout(confirmLabelLayout);
    formLayout->addWidget(passwordConfirmInput);
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
    mainLayout->addLayout(centerLayout);
    mainLayout->setAlignment(centerLayout, Qt::AlignCenter);

}

void SignUpPage::handleSignUpClicked() {
    QString email = emailInput->text().trimmed();
    QString password = passwordInput->text().trimmed();
    QString passwordConfirm = passwordConfirmInput->text().trimmed();

    if (email.isEmpty() || password.isEmpty() || passwordConfirm.isEmpty()) {
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

    if (password != passwordConfirm) {
        passwordConfirmInput->setStyleSheet(R"(
            background-color: #f5f7fa;
            padding: 10px;
            border: 2px solid red;
            border-radius: 0px;
        )");
        passwordMismatchLabel->show();
        return;
    } else {
        passwordConfirmInput->setStyleSheet(R"(
            background-color: #f5f7fa;
            padding: 10px;
            border: none;
            border-radius: 0px;
        )");
        passwordMismatchLabel->hide();
    }

    if (DBManager::instance().addUser(email, password)) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("가입 완료");
        msgBox.setText("가입이 완료되었습니다!");
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
    } else {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("가입 실패");
        msgBox.setText("이미 존재하는 이메일입니다!");
        msgBox.setIcon(QMessageBox::Critical);
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
    }

}
