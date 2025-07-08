#include "settingsdialog.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

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
    networkLayout->addRow("API URL :", apiUrlEdit);
    networkLayout->addRow("Port :", portEdit);
    networkLayout->addRow(autoConnectCheck);

    QWidget *networkPage = new QWidget;
    networkPage->setLayout(networkLayout);

    // 🎥 카메라 설정 페이지
    brightnessSlider = new ClickableSlider(Qt::Horizontal, this);
    brightnessSlider->setRange(0, 100);
    brightnessSlider->setTickInterval(1);
    contrastSlider = new ClickableSlider(Qt::Horizontal, this);
    contrastSlider->setRange(0, 100);
    contrastSlider->setTickInterval(1);
    exposureSlider = new ClickableSlider(Qt::Horizontal, this);
    exposureSlider->setRange(0, 100);
    exposureSlider->setTickInterval(1);
    saturationSlider = new ClickableSlider(Qt::Horizontal, this);
    saturationSlider->setRange(0, 100);
    saturationSlider->setTickInterval(1);

    QFormLayout *cameraLayout = new QFormLayout;
    cameraLayout->setVerticalSpacing(20);
    cameraLayout->setHorizontalSpacing(15);
    cameraLayout->setContentsMargins(10, 10, 10, 10);

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
    updateBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #f37321;     /* 밝은 파란색 배경 */
            color: white;                  /* 흰색 텍스트 */
            border: 1px solid #303030;
            border-radius: 10px;           /* 둥근 모서리 */
            font-size: 13px;
            padding: 5px;
        }
        QPushButton:hover {
            background-color: #f89b6c;     /* 호버 시 진한 파랑 */
        }
        QPushButton:pressed {
            background-color: #fbb584;     /* 눌렀을 때 더 진한 파랑 */
        }
    )");
    updateBtn->setFixedSize(90, 25);
    connect(updateBtn, &QPushButton::clicked, this, &SettingsDialog::onUpdateClicked);

    //cancel
    cancelBtn = new QPushButton("Cancel", this);
    cancelBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #f37321;     /* 밝은 파란색 배경 */
            color: white;                  /* 흰색 텍스트 */
            border: 1px solid #303030;
            border-radius: 10px;           /* 둥근 모서리 */
            font-size: 13px;
            padding: 5px;
        }
        QPushButton:hover {
            background-color: #f89b6c;     /* 호버 시 진한 파랑 */
        }
        QPushButton:pressed {
            background-color: #fbb584;     /* 눌렀을 때 더 진한 파랑 */
        }
    )");
    cancelBtn->setFixedSize(90, 25);
    connect(cancelBtn, &QPushButton::clicked, this, &SettingsDialog::onCancelClicked);

    QVBoxLayout *rightLayout = new QVBoxLayout;

    QHBoxLayout *rightHLayout = new QHBoxLayout;
    rightHLayout->setContentsMargins(8, 5, 8, 5);
    rightHLayout->addStretch();
    rightHLayout->addWidget(cancelBtn);
    rightHLayout->addWidget(updateBtn);

    rightLayout->addWidget(stackedPages);
    rightLayout->addLayout(rightHLayout);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(pageSelector);
    mainLayout->addLayout(rightLayout);

    setLayout(mainLayout);
    setWindowTitle("Settings");
    resize(600, 400);  // 기본 크기 설정

    // 설정 복원용 값 저장
    originalApiUrl = apiUrlEdit->text();
    originalPort = portEdit->text().toUInt();
    originalAutoConnect = autoConnectCheck->isChecked();
    originalBrightness = brightnessSlider->value();
    originalContrast = contrastSlider->value();
    originalExposure = exposureSlider->value();
    originalSaturation = saturationSlider->value();
}

void SettingsDialog::onPageChanged(int index) {
    if (index == 3) {
        this->close();  // 🏚 Home 선택 시 설정 창 닫기
    } else {
        stackedPages->setCurrentIndex(index);
    }
}

void SettingsDialog::onUpdateClicked() {
    if (stackedPages->currentIndex() == 0) {
        emit configUpdated();
    } else if (stackedPages->currentIndex() == 1) {
        emit cameraConfigUpdateRequested(
            brightnessSlider->value(),
            contrastSlider->value(),
            exposureSlider->value(),
            saturationSlider->value()
            );
    }
    accept();
}

void SettingsDialog::onCancelClicked() {
    // 설정 복원
    apiUrlEdit->setText(originalApiUrl);
    portEdit->setText(QString::number(originalPort));
    autoConnectCheck->setChecked(originalAutoConnect);

    brightnessSlider->setValue(originalBrightness);
    contrastSlider->setValue(originalContrast);
    exposureSlider->setValue(originalExposure);
    saturationSlider->setValue(originalSaturation);

    reject();
}


ClickableSlider::ClickableSlider(Qt::Orientation orientation, QWidget *parent)
    : QSlider(orientation, parent)
{
    setMinimumHeight(30);
}

void ClickableSlider::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        int sliderMin = this->minimum();
        int sliderMax = this->maximum();
        int newValue;

        if (orientation() == Qt::Horizontal) {
            int clickPos = event->x();
            int sliderWidth = this->width();
            double ratio = static_cast<double>(clickPos) / sliderWidth;
            ratio = std::min(std::max(ratio, 0.0), 1.0);
            newValue = sliderMin + static_cast<int>(ratio * (sliderMax - sliderMin));
        } else {
            int clickPos = event->y();
            int sliderHeight = this->height();
            double ratio = static_cast<double>(sliderHeight - clickPos) / sliderHeight;
            ratio = std::min(std::max(ratio, 0.0), 1.0);
            newValue = sliderMin + static_cast<int>(ratio * (sliderMax - sliderMin));
        }

        this->setValue(newValue);
    }

    QSlider::mousePressEvent(event);
}

void ClickableSlider::paintEvent(QPaintEvent *event)
{
    QSlider::paintEvent(event);

    QPainter painter(this);
    QStyleOptionSlider opt;
    initStyleOption(&opt);

    QRect handleRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);

    // 텍스트 설정
    QString valueText = QString::number(value());
    QFont font;
    font.setPointSize(8);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.setRenderHint(QPainter::Antialiasing);

    // 위치: 우측 상단 (슬라이더 전체 기준)
    int textX = width() - 35;
    int textY = 15;

    painter.drawText(QRect(textX, textY, 30, 20), Qt::AlignRight, valueText);
}

// ✅ 네트워크 설정 getter
QString SettingsDialog::getApiUrl() const {
    return apiUrlEdit->text();
}

quint16 SettingsDialog::getPort() const {
    return portEdit->text().toUShort();
}

bool SettingsDialog::getAutoConnect() const {
    return autoConnectCheck->isChecked();
}

// ✅ 카메라 설정 setter
void SettingsDialog::setBrightness(int value) {
    brightnessSlider->setValue(value);
}
void SettingsDialog::setContrast(int value) {
    contrastSlider->setValue(value);
}
void SettingsDialog::setExposure(int value) {
    exposureSlider->setValue(value);
}
void SettingsDialog::setSaturation(int value) {
    saturationSlider->setValue(value);
}

