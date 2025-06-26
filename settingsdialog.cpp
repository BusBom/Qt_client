#include "settingsdialog.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    // 💡 왼쪽 사이드바
    pageSelector = new QListWidget(this);
    pageSelector->addItem("🌐 네트워크 설정");
    pageSelector->addItem("🎥 카메라 설정");
    pageSelector->addItem("🌙 절전모드 설정");
    pageSelector->addItem("🏚 Home");
    pageSelector->setFixedWidth(150);
    pageSelector->setStyleSheet("background-color: #2a2a2a; color: white;");

    connect(pageSelector, &QListWidget::currentRowChanged, this, &SettingsDialog::onPageChanged);

    // 🌐 네트워크 설정 페이지
    apiUrlEdit = new QLineEdit(this);
    portEdit = new QLineEdit(this);
    autoConnectCheck = new QCheckBox("Auto Connect", this);

    QFormLayout *networkLayout = new QFormLayout;
    networkLayout->addRow("API URL:", apiUrlEdit);
    networkLayout->addRow("Port:", portEdit);
    networkLayout->addRow(autoConnectCheck);
    QWidget *networkPage = new QWidget;
    networkPage->setLayout(networkLayout);

    // 🎥 카메라 설정 페이지
    brightnessSlider = new QSlider(Qt::Horizontal, this);
    contrastSlider = new QSlider(Qt::Horizontal, this);
    exposureSlider = new QSlider(Qt::Horizontal, this);
    saturationSlider = new QSlider(Qt::Horizontal, this);

    QFormLayout *cameraLayout = new QFormLayout;
    cameraLayout->addRow("Brightness:", brightnessSlider);
    cameraLayout->addRow("Contrast:", contrastSlider);
    cameraLayout->addRow("Exposure:", exposureSlider);
    cameraLayout->addRow("Saturation:", saturationSlider);
    QWidget *cameraPage = new QWidget;
    cameraPage->setLayout(cameraLayout);

    // 🌙 절전모드 설정 페이지
    sleepStartEdit = new QTimeEdit(this);
    sleepEndEdit = new QTimeEdit(this);

    QFormLayout *sleepLayout = new QFormLayout;
    sleepLayout->addRow("절전모드 시작 시각:", sleepStartEdit);
    sleepLayout->addRow("절전모드 종료 시각:", sleepEndEdit);
    QWidget *sleepPage = new QWidget;
    sleepPage->setLayout(sleepLayout);

    // ✅ Stacked Widget 페이지 등록
    stackedPages = new QStackedWidget(this);
    stackedPages->addWidget(networkPage);  // index 0
    stackedPages->addWidget(cameraPage);   // index 1
    stackedPages->addWidget(sleepPage);    // index 2
    stackedPages->addWidget(new QWidget()); // index 3 - 홈 (비워두기)

    // ⚙️ 업데이트 버튼
    updateBtn = new QPushButton("Update", this);
    connect(updateBtn, &QPushButton::clicked, this, &SettingsDialog::onUpdateClicked);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(stackedPages);
    rightLayout->addWidget(updateBtn);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(pageSelector);
    mainLayout->addLayout(rightLayout);

    setLayout(mainLayout);
    setWindowTitle("Settings");
    resize(600, 400);  // 기본 크기 설정
}

void SettingsDialog::onPageChanged(int index) {
    if (index == 3) {
        this->close();  // 🏚 Home 선택 시 설정 창 닫기
    } else {
        stackedPages->setCurrentIndex(index);
    }
}

void SettingsDialog::onUpdateClicked() {
    // TODO: 실제 설정 저장 & 서버로 전송
    emit configUpdated();
    accept();
}

