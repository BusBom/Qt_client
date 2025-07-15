#include "settingsdialog.h"
#include <QLocalSocket>               // ✅ 유닉스 도메인 소켓
#define SOCKET_PATH "/tmp/camera_socket"  // ✅ 소켓 경로 정의
#include <QMediaPlayer>       // ✅ 영상 재생용
#include <QVideoWidget>       // ✅ 영상 표시용 위젯
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
    netManager = new QNetworkAccessManager(this);
    unixSocket = new QLocalSocket(this);

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

    QWidget *formWrapper = new QWidget;
    formWrapper->setLayout(cameraLayout);

    applyBtn = new QPushButton("Apply");
    applyBtn->setStyleSheet("background-color: #f37321; color: white; border-radius: 10px;");
    applyBtn->setFixedSize(90, 25);

    QHBoxLayout *applyLayout = new QHBoxLayout;  // ✅ Apply 버튼 오른쪽 정렬 배치
    applyLayout->addStretch();
    applyLayout->addWidget(applyBtn);
    applyLayout->setContentsMargins(0, 5, 15, 5);

    cameraLayoutContainer = new QVBoxLayout;

    cameraLayoutContainer->addWidget(formWrapper);
    cameraLayoutContainer->addLayout(applyLayout); // ✅ 슬라이더 바로 아래 오른쪽에 Apply 버튼 배치

    originalFrame = new QLabel("원본 영상");
    originalFrame->setFixedSize(320, 240);
    originalFrame->setStyleSheet("background-color: black; border: 1px solid gray;");
    originalFrame->setAlignment(Qt::AlignCenter);

    previewVideo = new QVideoWidget;
    previewVideo->setFixedSize(320, 240);
    previewVideo->setStyleSheet("background-color: black; border: 1px solid gray;");
    previewVideo->setAspectRatioMode(Qt::KeepAspectRatio);

    previewPlayer = new QMediaPlayer(this);
    previewPlayer->setVideoOutput(previewVideo);


    QHBoxLayout *previewLayout = new QHBoxLayout;  // ✅ 두 프레임 나란히 동일 크기, 같은 높이 배치
    previewLayout->addWidget(originalFrame);
    previewLayout->addWidget(previewVideo);

    cameraLayoutContainer->addLayout(previewLayout);

    connect(applyBtn, &QPushButton::clicked, this, [=]() {
        QJsonObject cameraObj;
        cameraObj["brightness"] = brightnessSlider->value();
        cameraObj["contrast"] = contrastSlider->value();
        cameraObj["exposure"] = exposureSlider->value();
        cameraObj["saturation"] = saturationSlider->value();
        cameraObj["preview"] = true;

        QJsonObject body;
        body["camera"] = cameraObj;

        QNetworkRequest request(QUrl("http://192.168.0.64/cgi-bin/config.cgi"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QNetworkReply *reply = netManager->post(request, QJsonDocument(body).toJson());

        connect(reply, &QNetworkReply::finished, this, [=]() {
            reply->deleteLater();

            // ✅ 실제 영상 스트림 주소로 수정 필요
            QUrl previewStreamUrl("http://192.168.0.64/preview_stream");  // 예시 URL
            previewPlayer->stop();   // 혹시 재생 중이면 중단
            previewPlayer->setSource(previewStreamUrl);
            previewPlayer->play();
        });
    });


    QWidget *cameraPage = new QWidget;
    cameraPage->setLayout(cameraLayoutContainer);

    // 🌙 절전모드 설정 페이지
    sleepStartEdit = new QTimeEdit(this);
    sleepEndEdit = new QTimeEdit(this);
    sleepStartEdit->setDisplayFormat("HH:mm");
    sleepEndEdit->setDisplayFormat("HH:mm");
    sleepStartEdit->setTime(QTime::fromString("01:00", "HH:mm"));
    sleepEndEdit->setTime(QTime::fromString("05:00", "HH:mm"));

    QFormLayout *sleepLayout = new QFormLayout;
    sleepLayout->addRow("절전모드 시작 시각:", sleepStartEdit);
    sleepLayout->addRow("절전모드 종료 시각:", sleepEndEdit);
    QWidget *sleepPage = new QWidget;
    sleepPage->setLayout(sleepLayout);

    stackedPages = new QStackedWidget(this);
    stackedPages->addWidget(networkPage);
    stackedPages->addWidget(cameraPage);
    stackedPages->addWidget(sleepPage);
    stackedPages->addWidget(new QWidget());

    updateBtn = new QPushButton("Update", this);
    updateBtn->setStyleSheet("background-color: #f37321; color: white; border-radius: 10px;");
    updateBtn->setFixedSize(90, 25);
    connect(updateBtn, &QPushButton::clicked, this, &SettingsDialog::onUpdateClicked);

    cancelBtn = new QPushButton("Cancel", this);
    cancelBtn->setStyleSheet("background-color: #f37321; color: white; border-radius: 10px;");
    cancelBtn->setFixedSize(90, 25);
    connect(cancelBtn, &QPushButton::clicked, this, &SettingsDialog::onCancelClicked);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    QHBoxLayout *rightHLayout = new QHBoxLayout;
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
    resize(900, 600);

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

        if (index == 1) {
            if (unixSocket->isOpen()) {
                unixSocket->abort();  // 이전 연결 닫기
            }

            unixSocket->connectToServer(SOCKET_PATH);  // 경로는 #define으로 사용

            QByteArray buffer;
            connect(unixSocket, &QLocalSocket::readyRead, this, [=]() mutable {
                buffer.append(unixSocket->readAll());
                // JPEG 종료 마커 감지 (0xFF 0xD9)
                if (buffer.contains("\xFF\xD9")) {
                    QPixmap pix;
                    pix.loadFromData(buffer);
                    originalFrame->setPixmap(pix.scaled(originalFrame->size(), Qt::KeepAspectRatio));
                    buffer.clear();
                    unixSocket->disconnectFromServer();  // 한 번 수신 후 종료
                }
            });
        }

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
