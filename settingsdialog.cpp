#include "settingsdialog.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>
#include <QPainter>
#include <QStyleOptionSlider>
#include <QMouseEvent>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QCheckBox>
#include <QStackedWidget>
#include <QTimeEdit>
#include <QLabel>
#include <QDebug>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    netManager = new QNetworkAccessManager(this);

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
    cameraLayout->setContentsMargins(10, 10, 10, 0);

    cameraLayout->addRow("Brightness:", brightnessSlider);
    cameraLayout->addRow("Contrast:", contrastSlider);
    cameraLayout->addRow("Exposure:", exposureSlider);
    cameraLayout->addRow("Saturation:", saturationSlider);

    QWidget *formWrapper = new QWidget;
    formWrapper->setLayout(cameraLayout);

    applyBtn = new QPushButton("Apply");
    applyBtn->setStyleSheet("background-color: #f37321; color: white; border-radius: 10px;");
    applyBtn->setFixedSize(90, 25);

    QHBoxLayout *applyLayout = new QHBoxLayout;
    applyLayout->addStretch();
    applyLayout->addWidget(applyBtn);
    applyLayout->setContentsMargins(0, 0, 15, 5);

    cameraLayoutContainer = new QVBoxLayout;
    cameraLayoutContainer->setSpacing(0);
    cameraLayoutContainer->addWidget(formWrapper);
    cameraLayoutContainer->addLayout(applyLayout);

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

    QHBoxLayout *previewLayout = new QHBoxLayout;
    previewLayout->addWidget(originalFrame);
    previewLayout->addWidget(previewVideo);
    cameraLayoutContainer->addLayout(previewLayout);

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

    // ✅ Apply 버튼 클릭 시 프리뷰 요청
    connect(applyBtn, &QPushButton::clicked, this, [=]() {
        QJsonObject cameraObj;
        cameraObj["brightness"] = brightnessSlider->value();
        cameraObj["contrast"] = contrastSlider->value();
        cameraObj["exposure"] = exposureSlider->value();
        cameraObj["saturation"] = saturationSlider->value();
        cameraObj["preview"] = true;

        QJsonObject body;
        body["camera"] = cameraObj;

        QNetworkRequest request(QUrl("http://192.168.0.59/cgi-bin/config.cgi"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QNetworkReply *reply = netManager->post(request, QJsonDocument(body).toJson());

        connect(reply, &QNetworkReply::finished, this, [=]() {
            reply->deleteLater();

            QByteArray responseData = reply->readAll();

            // ✅ (1) 먼저 JSON 에러 응답인지 검사
            if (responseData.startsWith("{")) {
                QJsonParseError parseError;
                QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);
                if (!parseError.error && doc.isObject() && doc.object().value("result") == "error") {
                    qWarning() << QString("❌ Apply - 서버 오류 응답: %1")
                                      .arg(doc.object().value("msg").toString());
                    return;
                }
            }

            // ✅ (2) JPEG 이미지 응답 처리
            QPixmap pix;
            if (pix.loadFromData(responseData)) {
                originalFrame->setPixmap(pix.scaled(originalFrame->size(), Qt::KeepAspectRatio));
                qInfo() << "✅ Apply - 원본 이미지 렌더링 완료";
            } else {
                qWarning() << "❌ Apply - JPEG 응답 파싱 실패";
            }

            // ✅ (3) RTSP 프리뷰 재생
            QUrl previewStreamUrl("rtsp://192.168.0.59:8554/stream");
            previewPlayer->stop();
            previewPlayer->setSource(previewStreamUrl);
            previewPlayer->play();

            // 🔍 스트리밍 실패 시 디버깅 메시지
            connect(previewPlayer, &QMediaPlayer::errorOccurred, this, [](QMediaPlayer::Error err) {
                qWarning() << "❌ Preview RTSP 스트림 오류 발생:" << err;
            });
        });
    });
}

void SettingsDialog::onPageChanged(int index) {
    if (index == 3) {
        close();
        return;
    }
    stackedPages->setCurrentIndex(index);
}

void SettingsDialog::onUpdateClicked() {
    if (stackedPages->currentIndex() == 1) {
        QJsonObject cameraObj {
            {"brightness", brightnessSlider->value()},
            {"contrast", contrastSlider->value()},
            {"exposure", exposureSlider->value()},
            {"saturation", saturationSlider->value()},
            {"preview", false}
        };
        QNetworkRequest req(QUrl("http://192.168.0.59/cgi-bin/config.cgi"));
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        netManager->post(req, QJsonDocument(QJsonObject{{"camera", cameraObj}}).toJson());
    }
    accept();
}

void SettingsDialog::onCancelClicked() {
    apiUrlEdit->setText(originalApiUrl);
    portEdit->setText(QString::number(originalPort));
    autoConnectCheck->setChecked(originalAutoConnect);
    brightnessSlider->setValue(originalBrightness);
    contrastSlider->setValue(originalContrast);
    exposureSlider->setValue(originalExposure);
    saturationSlider->setValue(originalSaturation);
    reject();
}

// ---- ClickableSlider 구현 ----
ClickableSlider::ClickableSlider(Qt::Orientation o, QWidget *p) : QSlider(o, p) { setMinimumHeight(30); }

void ClickableSlider::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        double ratio = (orientation() == Qt::Horizontal) ?
                           static_cast<double>(event->x()) / width() :
                           static_cast<double>(height() - event->y()) / height();
        setValue(minimum() + static_cast<int>(std::clamp(ratio, 0.0, 1.0) * (maximum() - minimum())));
    }
    QSlider::mousePressEvent(event);
}

void ClickableSlider::paintEvent(QPaintEvent *e) {
    QSlider::paintEvent(e);
    QPainter painter(this);
    QStyleOptionSlider opt;
    initStyleOption(&opt);
    QString valueText = QString::number(value());
    painter.setFont(QFont("", 8));
    painter.setPen(Qt::white);
    painter.drawText(QRect(width() - 35, 15, 30, 20), Qt::AlignRight, valueText);
}

// --- Getters ---
QString SettingsDialog::getApiUrl() const { return apiUrlEdit->text(); }
quint16 SettingsDialog::getPort() const { return portEdit->text().toUShort(); }
bool SettingsDialog::getAutoConnect() const { return autoConnectCheck->isChecked(); }
void SettingsDialog::setBrightness(int v) { brightnessSlider->setValue(v); }
void SettingsDialog::setContrast(int v) { contrastSlider->setValue(v); }
void SettingsDialog::setExposure(int v) { exposureSlider->setValue(v); }
void SettingsDialog::setSaturation(int v) { saturationSlider->setValue(v); }
