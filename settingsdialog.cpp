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
#include <QJsonArray>
#include "roi_frame.h"
RoiFrame *roiCanvas;

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    netManager = new QNetworkAccessManager(this);

    pageSelector = new QListWidget(this);
    pageSelector->addItem("🌐 네트워크 설정");
    pageSelector->addItem("🎥 카메라 설정");
    pageSelector->addItem("🧭 ROI 설정");
    pageSelector->addItem("🌙 절전모드 설정");
    pageSelector->addItem("🏚 Home");
    pageSelector->setFixedWidth(150);
    pageSelector->setStyleSheet("background-color: #2a2a2a; color: white;");
    connect(pageSelector, &QListWidget::currentRowChanged, this, &SettingsDialog::onPageChanged);

    // 🌐 네트워크 설정
    apiUrlEdit = new QLineEdit(this);
    portEdit = new QLineEdit(this);
    autoConnectCheck = new QCheckBox("Auto Connect", this);
    QFormLayout *networkLayout = new QFormLayout;
    networkLayout->addRow("API URL :", apiUrlEdit);
    networkLayout->addRow("Port :", portEdit);
    networkLayout->addRow(autoConnectCheck);
    QWidget *networkPage = new QWidget;
    networkPage->setLayout(networkLayout);

    // 🎥 카메라 설정
    brightnessSlider = new ClickableSlider(Qt::Horizontal, this);
    brightnessSlider->setRange(0, 100);
    contrastSlider = new ClickableSlider(Qt::Horizontal, this);
    contrastSlider->setRange(0, 100);
    exposureSlider = new ClickableSlider(Qt::Horizontal, this);
    exposureSlider->setRange(0, 100);
    saturationSlider = new ClickableSlider(Qt::Horizontal, this);
    saturationSlider->setRange(0, 100);

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
    applyBtn->setStyleSheet("background-color: #2c3e50; color: white; border-radius: 10px; padding-top: 1px;");
    applyBtn->setFixedSize(65, 25);
    QHBoxLayout *applyLayout = new QHBoxLayout;
    applyLayout->addStretch();
    applyLayout->addWidget(applyBtn);
    applyLayout->setContentsMargins(0, 0, 15, 5);

    cameraLayoutContainer = new QVBoxLayout;
    cameraLayoutContainer->setSpacing(0);
    cameraLayoutContainer->addWidget(formWrapper);
    cameraLayoutContainer->addLayout(applyLayout);

    originalFrame = new QLabel("원본 영상");
    originalFrame->setFixedSize(350, 270);
    originalFrame->setStyleSheet("background-color: black; border: 1px solid gray;");
    originalFrame->setAlignment(Qt::AlignCenter);

    previewVideo = new QVideoWidget;
    previewVideo->setFixedSize(350, 270);
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

    // 🧭 ROI 설정
    roiCanvas = new RoiFrame(this);  // ✅ 새로 만든 RoiFrame 클래스 사용
    roiCanvas->setFixedSize(640, 360);
    roiCanvas->setStyleSheet("background-color: black; border: 1px solid gray;");
    roiCanvas->setAlignment(Qt::AlignCenter);

    // 가이드 라벨 추가
    QLabel *roiGuideLabel = new QLabel("※ 플랫폼 상 가장 앞 쪽이 1번 플랫폼\n※ LT, RT, RB, LB 순서로 ROI 지정", this);
    roiGuideLabel->setStyleSheet("color: lightgray; font-size: 15px;");
    roiGuideLabel->setAlignment(Qt::AlignLeft);

    QVBoxLayout *roiCanvasWithGuideLayout = new QVBoxLayout;
    roiCanvasWithGuideLayout->setSpacing(4);
    roiCanvasWithGuideLayout->addWidget(roiCanvas, 0, Qt::AlignTop);
    roiCanvasWithGuideLayout->addWidget(roiGuideLabel);


    platformCountLabel = new QLabel("플랫폼 개수: 0");
    platformCountLabel->setStyleSheet("color: white; font-weight: bold;");

    for (int i = 0; i < 4; ++i) {
        QLabel *label = new QLabel();
        label->setStyleSheet("color: white;");
        coordLabels.append(label);
    }

    QVBoxLayout *roiSideLayout = new QVBoxLayout;
    roiSideLayout->setContentsMargins(0, 7, 0, 0);
    roiSideLayout->addWidget(platformCountLabel);
    for (QLabel *label : coordLabels)
        roiSideLayout->addWidget(label);
    roiSideLayout->addStretch();

    QHBoxLayout *roiMainLayout = new QHBoxLayout;
    roiMainLayout->addLayout(roiCanvasWithGuideLayout);
    roiMainLayout->addSpacing(5);
    roiMainLayout->addLayout(roiSideLayout);

    // roiCanvas → roiPolygons 반영 및 UI 갱신 연결
    connect(roiCanvas, &RoiFrame::roiUpdated, this, [=]() {
        roiPolygons = roiCanvas->getRois();
        updateRoiDisplay();
    });

    QWidget *roiPage = new QWidget;
    roiPage->setLayout(roiMainLayout);

    // 🌙 절전모드
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

    // 스택 페이지 연결
    stackedPages = new QStackedWidget(this);
    stackedPages->addWidget(networkPage);
    stackedPages->addWidget(cameraPage);
    stackedPages->addWidget(roiPage);
    stackedPages->addWidget(sleepPage);
    stackedPages->addWidget(new QWidget());  // Home placeholder

    updateBtn = new QPushButton("Update", this);
    updateBtn->setStyleSheet("background-color: #f37321; color: white; border-radius: 10px; padding-top: 1px;");
    updateBtn->setFixedSize(90, 25);
    connect(updateBtn, &QPushButton::clicked, this, &SettingsDialog::onUpdateClicked);

    cancelBtn = new QPushButton("Cancel", this);
    cancelBtn->setStyleSheet("background-color: #f37321; color: white; border-radius: 10px; padding-top: 1px;");
    cancelBtn->setFixedSize(90, 25);
    connect(cancelBtn, &QPushButton::clicked, this, &SettingsDialog::onCancelClicked);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    QHBoxLayout *rightHLayout = new QHBoxLayout;
    rightHLayout->addStretch();
    rightHLayout->addWidget(cancelBtn);
    rightHLayout->addWidget(updateBtn);
    rightLayout->addWidget(stackedPages);
    rightHLayout->setContentsMargins(0, 0, 15, 15);  // 좌, 상, 우, 하
    rightLayout->addLayout(rightHLayout);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(pageSelector);
    mainLayout->addLayout(rightLayout);
    setLayout(mainLayout);
    setWindowTitle("Settings");
    resize(900, 560);

    // 설정값 백업
    originalApiUrl = apiUrlEdit->text();
    originalPort = portEdit->text().toUInt();
    originalAutoConnect = autoConnectCheck->isChecked();
    originalBrightness = brightnessSlider->value();
    originalContrast = contrastSlider->value();
    originalExposure = exposureSlider->value();
    originalSaturation = saturationSlider->value();

    // Apply 프리뷰 기능 연결
    connect(applyBtn, &QPushButton::clicked, this, [=]() {
        QJsonObject cameraObj {
            {"brightness", brightnessSlider->value()},
            {"contrast", contrastSlider->value()},
            {"exposure", exposureSlider->value()},
            {"saturation", saturationSlider->value()},
            {"preview", true}
        };
        QJsonObject body {{"camera", cameraObj}};
        QNetworkRequest req(QUrl("http://192.168.0.59/cgi-bin/config.cgi"));
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QNetworkReply *reply = netManager->post(req, QJsonDocument(body).toJson());

        connect(reply, &QNetworkReply::finished, this, [=]() {
            reply->deleteLater();
            QByteArray responseData = reply->readAll();
            QPixmap pix;
            if (pix.loadFromData(responseData)) {
                originalFrame->setPixmap(pix.scaled(originalFrame->size(), Qt::KeepAspectRatio));
                qInfo() << "✅ Apply - 원본 이미지 렌더링 완료";
            } else {
                qWarning() << "❌ Apply - JPEG 응답 파싱 실패";
            }

            QUrl streamUrl("rtsp://192.168.0.59:8554/stream");
            previewPlayer->stop();
            previewPlayer->setSource(streamUrl);
            previewPlayer->play();
            connect(previewPlayer, &QMediaPlayer::errorOccurred, this, [](QMediaPlayer::Error err) {
                qWarning() << "❌ Preview 스트림 오류:" << err;
            });
        });
    });
    pageSelector->setCurrentRow(0); // 네트워크 설정 탭을 기본 선택
    onPageChanged(0);               // Cancel 버튼 텍스트도 함께 초기화
}


// ROI 정보 표시용 함수 추가
void SettingsDialog::updateRoiDisplay() {
    platformCountLabel->setText(QString("플랫폼 개수: %1").arg(roiPolygons.size()));

    for (int i = 0; i < coordLabels.size(); ++i) {
        if (i < roiPolygons.size()) {
            const QVector<QPoint> &pts = roiPolygons[i];
            QString text = QString("좌표: (%1, %2)\n        (%3, %4)\n        (%5, %6)\n        (%7, %8)")
                               .arg(pts[0].x()).arg(pts[0].y())
                               .arg(pts[1].x()).arg(pts[1].y())
                               .arg(pts[2].x()).arg(pts[2].y())
                               .arg(pts[3].x()).arg(pts[3].y());
            coordLabels[i]->setText(text);
        } else if (i == 0) {
            coordLabels[i]->setText("좌표:");
        } else {
            coordLabels[i]->clear();
        }
    }
}


void SettingsDialog::onPageChanged(int index) {
    if (index == 4) {
        close();
        return;
    }
    stackedPages->setCurrentIndex(index);

    if (index == 2) {
        cancelBtn->setText("Reset ROI");

        // 이미지 캡처 받아오기
        QNetworkRequest imgReq(QUrl("http://192.168.0.33/cgi-bin/capture.cgi"));
        QNetworkReply *imgReply = netManager->get(imgReq);
        connect(imgReply, &QNetworkReply::finished, this, [=]() {
            imgReply->deleteLater();
            QPixmap pix;
            if (pix.loadFromData(imgReply->readAll())) {
                roiCanvas->setBackgroundImage(pix);  // roiCanvas에 QPixmap 설정
                qDebug() << "✅ 캡처 이미지 수신 및 적용 완료";
            } else {
                qWarning() << "❌ 캡처 이미지 로딩 실패";
            }
        });
    } else {
        cancelBtn->setText("Cancel");
    }
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

    // ROI 설정 탭
    if (stackedPages->currentIndex() == 2) {
        const double scaleX = 1280.0 / roiCanvas->width();
        const double scaleY = 720.0 / roiCanvas->height();

        QJsonArray stopRois;
        for (const auto &polygon : roiPolygons) {
            QJsonArray roiArray;
            for (const QPoint &pt : polygon) {
                QJsonArray ptArray;
                ptArray.append(static_cast<int>(pt.x() * scaleX));
                ptArray.append(static_cast<int>(pt.y() * scaleY));
                roiArray.append(ptArray);
            }
            stopRois.append(roiArray);
        }

        QJsonObject body;
        body["stop_rois"] = stopRois;

        QNetworkRequest req(QUrl("http://192.168.0.59/cgi-bin/roi-setup.cgi"));
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        netManager->post(req, QJsonDocument(body).toJson());

        qDebug() << "✅ ROI 설정 전송 (원본 좌표 기준):" << body;
    }


}

void SettingsDialog::onCancelClicked() {
    if (stackedPages->currentIndex() == 2) {
        // ROI 설정 탭: ROI만 초기화
        roiPolygons.clear();
        roiCanvas->clear();
        updateRoiDisplay();
    } else {
        // 나머지 탭: 원래대로 설정 복원하고 창 닫기
        apiUrlEdit->setText(originalApiUrl);
        portEdit->setText(QString::number(originalPort));
        autoConnectCheck->setChecked(originalAutoConnect);
        brightnessSlider->setValue(originalBrightness);
        contrastSlider->setValue(originalContrast);
        exposureSlider->setValue(originalExposure);
        saturationSlider->setValue(originalSaturation);
        reject();  // 설정창 닫기
    }
}

// ---- ClickableSlider 구현 ----
ClickableSlider::ClickableSlider(Qt::Orientation o, QWidget *p) : QSlider(o, p) { setMinimumHeight(32); }

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
    painter.drawText(QRect(width() - 35, -3, 30, 20), Qt::AlignRight, valueText);
}

// --- Getters ---
QString SettingsDialog::getApiUrl() const { return apiUrlEdit->text(); }
quint16 SettingsDialog::getPort() const { return portEdit->text().toUShort(); }
bool SettingsDialog::getAutoConnect() const { return autoConnectCheck->isChecked(); }
void SettingsDialog::setBrightness(int v) { brightnessSlider->setValue(v); }
void SettingsDialog::setContrast(int v) { contrastSlider->setValue(v); }
void SettingsDialog::setExposure(int v) { exposureSlider->setValue(v); }
void SettingsDialog::setSaturation(int v) { saturationSlider->setValue(v); }
