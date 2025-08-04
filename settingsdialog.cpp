#include "settingsdialog.h"
#include <QSslConfiguration>
#include <QSslCertificate>
#include <QSslKey>
#include <QSslSocket>
#include <QFile>
#include <QCoreApplication>
#include <QBuffer>
#include <QImageReader>
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
#include <QFrame>
#include "roi_frame.h"

RoiFrame *roiCanvas;

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    QFont hanwhaFont("hanwhaGothic EL");
    setFont(hanwhaFont);

    netManager = new QNetworkAccessManager(this);

    // 좌측 메뉴 리스트
    pageSelector = new QListWidget(this);
    pageSelector->addItem("🖧 네트워크 설정");
    pageSelector->addItem("⧉ 카메라 설정");
    pageSelector->addItem("✦ ROI 설정");
    pageSelector->addItem("☾ 절전모드 설정");
    pageSelector->setFixedWidth(150);
    pageSelector->setStyleSheet(R"(
        QListWidget {
            background-color: #1e1e1e;
            border: none;
            color: white;
            font-size: 13px;
            outline: 0;
        }
        QListWidget::item {
            padding: 14px 8px;
            border-bottom: 1px solid #333;
        }
        QListWidget::item:selected {
            color: #f37321;
            font-weight: bold;
            background-color: #1e1e1e;
        }
        QListWidget::item::focus {
            outline: none;
        }

        QListWidget::item:!active {
            outline: none;
        }

        QListWidget::item:selected:active {
            outline: none;
        }
    )");
    connect(pageSelector, &QListWidget::currentRowChanged, this, &SettingsDialog::onPageChanged);

    // 중앙 세로 구분선
    QFrame *vLine = new QFrame;
    vLine->setFrameShape(QFrame::VLine);
    vLine->setFrameShadow(QFrame::Plain);
    vLine->setStyleSheet("color: #444;");
    vLine->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    vLine->setLineWidth(1);

    auto makeSection = [](const QString &title, QWidget *content) {
        QLabel *titleLabel = new QLabel(title);
        titleLabel->setStyleSheet("color: white; font-size: 15px; font-weight: bold; margin-left: 8px; ");
        titleLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

        QFrame *hLine = new QFrame;
        hLine->setFrameShape(QFrame::HLine);
        hLine->setFrameShadow(QFrame::Sunken);
        hLine->setStyleSheet("background-color: #444; border: none;");
        hLine->setFixedHeight(1);

        content->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);

        QVBoxLayout *layout = new QVBoxLayout;
        layout->setContentsMargins(0, 10, 0, 0);
        layout->setSpacing(4);
        layout->addWidget(titleLabel);
        layout->addWidget(hLine);
        layout->addWidget(content);
        layout->addStretch();

        QWidget *wrapper = new QWidget;
        wrapper->setLayout(layout);
        return wrapper;
    };


    // 🌐 네트워크 설정
    apiUrlEdit = new QLineEdit(this);
    portEdit = new QLineEdit(this);
    autoConnectCheck = new QCheckBox("Auto Connect", this);

    QFormLayout *networkLayout = new QFormLayout;
    networkLayout->addRow("API URL:", apiUrlEdit);
    networkLayout->addRow("Port:", portEdit);
    networkLayout->addRow(autoConnectCheck);

    QWidget *networkWidget = new QWidget;
    networkWidget->setLayout(networkLayout);

    QWidget *networkPage = makeSection("네트워크 설정", networkWidget);


    // 🎥 카메라 설정
    brightnessSlider = new ClickableSlider(Qt::Horizontal, this);
    brightnessSlider->setRange(0, 100);
    contrastSlider = new ClickableSlider(Qt::Horizontal, this);
    contrastSlider->setRange(0, 100);
    exposureSlider = new ClickableSlider(Qt::Horizontal, this);
    exposureSlider->setRange(0, 100);
    saturationSlider = new ClickableSlider(Qt::Horizontal, this);
    saturationSlider->setRange(0, 100);

    QString sliderStyle = R"(
        QSlider::groove:horizontal {
            height: 4px;
            background: white;
            margin: 0px;
        }
        QSlider::handle:horizontal {
            background: rgba(200, 200, 200, 0.6);
            background: white;
            width: 8px;
            height: 18px;
            margin: -5px 0;
            border-radius: 0px;
        }
    )";
    for (ClickableSlider *slider : {
             brightnessSlider, contrastSlider, exposureSlider, saturationSlider
         }) {
        slider->setStyleSheet(sliderStyle);
    }


    QFormLayout *cameraForm = new QFormLayout;
    cameraForm->addRow("Brightness:", brightnessSlider);
    cameraForm->addRow("Contrast:", contrastSlider);
    cameraForm->addRow("Exposure:", exposureSlider);
    cameraForm->addRow("Saturation:", saturationSlider);

    applyBtn = new QPushButton("Apply");
    applyBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #3a3a3a;
            color: white;
            border: none;
            border-radius: 5px;
            font-size: 13px;
            padding: 4px 10px;
        }
        QPushButton:focus {
            outline: none;
        }
    )");

    applyBtn->setFixedSize(65, 25);

    QHBoxLayout *applyLayout = new QHBoxLayout;
    applyLayout->addStretch();
    applyLayout->addWidget(applyBtn);
    applyLayout->setContentsMargins(0, 0, 5, 0);

    // 📷 원본 영상
    originalFrame = new QLabel("원본 영상");
    originalFrame->setFixedSize(370, 270);
    originalFrame->setStyleSheet("background-color: black; border: 1px solid gray;");
    originalFrame->setAlignment(Qt::AlignCenter);

    // 📷 미리보기 영상
    previewVideo = new QVideoWidget;
    previewVideo->setFixedSize(370, 270);
    previewVideo->setStyleSheet("background-color: black; border: 1px solid gray;");

    previewPlayer = new QMediaPlayer(this);
    previewPlayer->setVideoOutput(previewVideo);

    // 📷 영상 두 개를 좌우로 딱 붙임
    QHBoxLayout *previewLayout = new QHBoxLayout;
    previewLayout->setSpacing(0);
    previewLayout->setContentsMargins(0, 0, 0, 0);
    previewLayout->addWidget(originalFrame);
    previewLayout->addWidget(previewVideo);

    // 전체 카메라 페이지 구성
    QVBoxLayout *cameraLayout = new QVBoxLayout;
    cameraLayout->addLayout(cameraForm);
    cameraLayout->addLayout(applyLayout);
    cameraLayout->addLayout(previewLayout);

    QWidget *cameraContent = new QWidget;
    cameraContent->setLayout(cameraLayout);
    QWidget *cameraPage = makeSection("카메라 설정", cameraContent);

    // 🧭 ROI 설정
    roiCanvas = new RoiFrame(this);
    roiCanvas->setFixedSize(640, 360);
    roiCanvas->setStyleSheet("background-color: black; border: 1px solid gray;");
    roiCanvas->setAlignment(Qt::AlignCenter);

    QLabel *roiGuideLabel = new QLabel("※ 플랫폼 상 가장 앞 쪽이 1번 플랫폼\n※ LT, RT, RB, LB 순서로 ROI 지정", this);
    roiGuideLabel->setStyleSheet("color: lightgray; font-size: 15px;");

    QVBoxLayout *roiLeft = new QVBoxLayout;
    roiLeft->addWidget(roiCanvas);
    roiLeft->addWidget(roiGuideLabel);

    platformCountLabel = new QLabel("플랫폼 개수: 0");
    platformCountLabel->setStyleSheet("color: white; font-weight: bold;");
    QVBoxLayout *roiRight = new QVBoxLayout;
    roiRight->addWidget(platformCountLabel);
    for (int i = 0; i < 4; ++i) {
        QLabel *label = new QLabel;
        label->setStyleSheet("color: white;");
        coordLabels.append(label);
        roiRight->addWidget(label);
    }
    roiRight->addStretch();

    QHBoxLayout *roiBody = new QHBoxLayout;
    roiBody->addLayout(roiLeft);
    roiBody->addSpacing(10);
    roiBody->addLayout(roiRight);

    QWidget *roiContent = new QWidget;
    roiContent->setLayout(roiBody);
    QWidget *roiPage = makeSection("ROI 설정", roiContent);

    connect(roiCanvas, &RoiFrame::roiUpdated, this, [=]() {
        roiPolygons = roiCanvas->getRois();
        updateRoiDisplay();
    });

    // 🌙 절전모드 설정
    sleepStartEdit = new QTimeEdit(this);
    sleepEndEdit = new QTimeEdit(this);
    sleepStartEdit->setDisplayFormat("HH:mm");
    sleepEndEdit->setDisplayFormat("HH:mm");
    sleepStartEdit->setTime(QTime::fromString("01:00", "HH:mm"));
    sleepEndEdit->setTime(QTime::fromString("05:00", "HH:mm"));

    QFormLayout *sleepForm = new QFormLayout;
    sleepForm->addRow("절전모드 시작 시각:", sleepStartEdit);
    sleepForm->addRow("절전모드 종료 시각:", sleepEndEdit);

    QWidget *sleepContent = new QWidget;
    sleepContent->setLayout(sleepForm);
    QWidget *sleepPage = makeSection("절전모드 설정", sleepContent);

    // ✅ stackedPages에 각 페이지 추가
    stackedPages = new QStackedWidget(this);
    stackedPages->addWidget(networkPage);
    stackedPages->addWidget(cameraPage);
    stackedPages->addWidget(roiPage);
    stackedPages->addWidget(sleepPage);

    // ✅ 하단 버튼 영역
    updateBtn = new QPushButton("Update", this);
    cancelBtn = new QPushButton("Cancel", this);
    QString flatBtnStyle = R"(
        QPushButton {
            background-color: #f37321;
            color: white;
            border: none;
            border-radius: 5px;
            padding: 0px;
            font-size: 13px;
            height: 25px;
        }
        QPushButton:focus {
            outline: none;
        }
    )";

    for (QPushButton *btn : {updateBtn, cancelBtn}) {
        btn->setStyleSheet(flatBtnStyle);
        btn->setFixedSize(90, 25);
    }

    connect(updateBtn, &QPushButton::clicked, this, &SettingsDialog::onUpdateClicked);
    connect(cancelBtn, &QPushButton::clicked, this, &SettingsDialog::onCancelClicked);

    QHBoxLayout *bottomBtns = new QHBoxLayout;
    bottomBtns->addStretch();
    bottomBtns->addWidget(cancelBtn);
    bottomBtns->addWidget(updateBtn);
    bottomBtns->setContentsMargins(0, 0, 15, 15);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(stackedPages);
    rightLayout->addLayout(bottomBtns);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(pageSelector);
    mainLayout->addWidget(vLine);
    mainLayout->addLayout(rightLayout);

    setLayout(mainLayout);
    setWindowTitle("Settings");
    resize(950, 560);

    // 초기 설정값 백업
    originalApiUrl = apiUrlEdit->text();
    originalPort = portEdit->text().toUInt();
    originalAutoConnect = autoConnectCheck->isChecked();
    originalBrightness = brightnessSlider->value();
    originalContrast = contrastSlider->value();
    originalExposure = exposureSlider->value();
    originalSaturation = saturationSlider->value();

    pageSelector->setCurrentRow(0);
    onPageChanged(0);

}

QSslConfiguration SettingsDialog::createSslConfig() {
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();

    qDebug() << "📄 인증서/키/CA 경로:"
             << QCoreApplication::applicationDirPath() + "/client.cert.pem"
             << QCoreApplication::applicationDirPath() + "/client.key.pem"
             << QCoreApplication::applicationDirPath() + "/ca.cert.pem";


    // 클라이언트 인증서
    QSslCertificate cert;
    QFile certFile(QCoreApplication::applicationDirPath() + "/client.cert.pem");
    if (certFile.open(QIODevice::ReadOnly))
        cert = QSslCertificate(certFile.readAll(), QSsl::Pem);
    else
        qWarning() << " 클라이언트 인증서 로드 실패";

    // 클라이언트 키
    QSslKey key;
    QFile keyFile(QCoreApplication::applicationDirPath() + "/client.key.pem");
    if (keyFile.open(QIODevice::ReadOnly))
        key = QSslKey(keyFile.readAll(), QSsl::Rsa, QSsl::Pem);
    else
        qWarning() << " 클라이언트 개인키 로드 실패";

    // CA 인증서
    QList<QSslCertificate> caCerts;
    QFile caFile(QCoreApplication::applicationDirPath() + "/ca.cert.pem");
    if (caFile.open(QIODevice::ReadOnly))
        caCerts = QSslCertificate::fromData(caFile.readAll(), QSsl::Pem);
    else
        qWarning() << " CA 인증서 로드 실패";

    sslConfig.setLocalCertificate(cert);
    sslConfig.setPrivateKey(key);
    sslConfig.setCaCertificates(caCerts);
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyPeer);
    sslConfig.setProtocol(QSsl::TlsV1_2);

    return sslConfig;
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

        // SSL + 인증서 설정
        QNetworkRequest imgReq(QUrl("https://192.168.0.50/cgi-bin/capture.cgi"));
        //imgReq.setSslConfiguration(createSslConfig());
        QSslConfiguration config = QSslConfiguration::defaultConfiguration();
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        imgReq.setSslConfiguration(config);


        QNetworkReply *imgReply = netManager->get(imgReq);
        connect(imgReply, &QNetworkReply::finished, this, [=]() {
            imgReply->deleteLater();
            QByteArray responseData = imgReply->readAll();
            qDebug() << "📦 수신 데이터 크기:" << responseData.size();

            // QImageReader로 MIME 명시 (JPEG 지정)
            QBuffer buffer(&responseData);
            buffer.open(QIODevice::ReadOnly);
            QImageReader reader(&buffer, "JPEG");
            QImage img = reader.read();

            if (!img.isNull()) {
                roiCanvas->setBackgroundImage(QPixmap::fromImage(img));
                qDebug() << "✅ 캡처 이미지 수신 및 적용 완료";
            } else {
                qDebug() << "❌ 캡처 이미지 로딩 실패";
                QFile f("capture_debug_failed.jpg");
                f.open(QIODevice::WriteOnly);
                f.write(responseData);
                f.close();
                qDebug() << "📄 capture_debug_failed.jpg 저장 완료";
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
        QNetworkRequest req(QUrl("https://192.168.0.82/cgi-bin/config.cgi"));
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        //req.setSslConfiguration(createSslConfig());  // ✅ SSL 인증 설정
        QSslConfiguration config = QSslConfiguration::defaultConfiguration();
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        req.setSslConfiguration(config);


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

        QNetworkRequest req(QUrl("https://192.168.0.82/cgi-bin/roi-setup.cgi"));
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        //req.setSslConfiguration(createSslConfig());  // ✅ SSL 인증 설정
        QSslConfiguration config = QSslConfiguration::defaultConfiguration();
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        req.setSslConfiguration(config);


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
