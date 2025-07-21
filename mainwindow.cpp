#include "mainwindow.h"
#include "settingsdialog.h"
#include "ui_mainwindow.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QPixmap>
#include <QLabel>
#include <QFrame>
#include <QSpacerItem>
#include <QStyle>
#include <QComboBox>
#include <QTimer>
#include <QThread>
#include <QCoreApplication>
#include <opencv2/opencv.hpp>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QNetworkAccessManager>
#include <QPushButton>
#include <QComboBox>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkReply>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(1280, 720);
    setStyleSheet("background-color: #1e1e1e; color: white;");
    QIcon icon(PATH + "/images/bus_face.png");
    setWindowIcon(icon);

    setupUI();
    setupConnections();

    settingsDlg = new SettingsDialog(this);
    networkManager = new QNetworkAccessManager(this);
    /*
    pollTimer = new QTimer(this);
    connect(pollTimer, &QTimer::timeout, this, &MainWindow::updateConnectionStatus);
    pollTimer->start(3000);
    */

    // ✅ SettingsDialog 시그널 연결
    connect(settingsDlg, &SettingsDialog::configUpdated, this, [=]() {
        // ✅ 네트워크 설정 반영
        apiUrlBase = settingsDlg->getApiUrl();
        apiPort = settingsDlg->getPort();
        autoConnect = settingsDlg->getAutoConnect();
    });

    connect(settingsDlg, &SettingsDialog::cameraConfigUpdateRequested, this, [=](int brightness, int contrast, int exposure, int saturation) {
        // ✅ 카메라 설정 서버 전송
        QJsonObject cameraObj;
        cameraObj["brightness"] = brightness;
        cameraObj["contrast"] = contrast;
        cameraObj["exposure"] = exposure;
        cameraObj["saturation"] = saturation;

        QJsonObject body;
        body["camera"] = cameraObj;

        QNetworkRequest request(QUrl("http://192.168.0.59/cgi-bin/config.cgi"));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QNetworkReply *reply = networkManager->post(request, QJsonDocument(body).toJson());

        connect(reply, &QNetworkReply::finished, this, [=]() {
            QByteArray response = reply->readAll();
            qDebug() << "📷 카메라 설정 응답:" << response;  // ✅ 로그 추가

            QJsonDocument doc = QJsonDocument::fromJson(response);
            if (!doc.isNull() && doc.isObject()) {
                QJsonObject resCam = doc.object().value("camera").toObject();
                settingsDlg->setBrightness(resCam.value("brightness").toInt());
                settingsDlg->setContrast(resCam.value("contrast").toInt());
                settingsDlg->setExposure(resCam.value("exposure").toInt());
                settingsDlg->setSaturation(resCam.value("saturation").toInt());

                // ✅ 설정 반영 확인용 로그도 추가!
                qDebug() << "✅ 설정 반영됨 -> 밝기:" << resCam.value("brightness").toInt()
                         << ", 명암:" << resCam.value("contrast").toInt()
                         << ", 노출:" << resCam.value("exposure").toInt()
                         << ", 채도:" << resCam.value("saturation").toInt();
            }
            reply->deleteLater();
        });
    });

    // ✅ 버스 정보 실시간 갱신용 타이머
    busTimer = new QTimer(this);
    connect(busTimer, &QTimer::timeout, this, &MainWindow::fetchBusData);
    busTimer->start(1000);

    // ✅ 기본값 초기화
    apiUrlBase = "http://192.168.0.59/cgi-bin/sequence.cgi";
    apiPort = 80;
    autoConnect = false;
}

void MainWindow::setupUI() {
    // 🚍 Title + Icon
    QLabel *titleImgLabel = new QLabel("<img src='" + PATH + "/images/bus_face.png' width=32 height=32>");
    QLabel *titleTextLabel = new QLabel("<b style='font-size:25px; color: white;'> Live Dashboard</b>");
    titleTextLabel->setContentsMargins(0, 0, 0, 2);

    stopSelector = new QComboBox(this);
    stopSelector->setStyleSheet(R"(
        QComboBox {
            font-size: 12pt;
            background-color: #2e2e2e;
            color: white;
            border-radius: 30px;
            padding: 4px;
        }
        QComboBox QAbstractItemView {
            font-size: 12pt;
        }
    )");
    stopSelector->addItems({"래미안아파트.파이낸셜뉴스", "신분당선 강남역", "지하철2호선 강남역", "논현역"});

    statusRpi = new QLabel("Server: 🔴");
    statusRpi->setStyleSheet("background-color: #313131;");
    statusCam = new QLabel("Camera: 🔴");
    statusCam->setStyleSheet("background-color: #313131;");
    statusStm32 = new QLabel("Display: 🔴");
    statusStm32->setStyleSheet("background-color: #313131;");

    statusRpi->setFixedWidth(70);
    statusCam->setFixedWidth(70);
    statusStm32->setFixedWidth(70);

    QWidget *statusWidget = new QWidget;             // 또는 QFrame *statusWidget = new QFrame;
    statusWidget->setObjectName("statusWidget");
    statusWidget->setStyleSheet(R"(
    #statusWidget {
        background-color: #313131;
        border: 1px solid #313131;
        border-radius: 15px;
        padding-left: 15px;
        padding-right: 15px;

    }

    QLabel {
        qproperty-alignment: 'AlignCenter';  /* QLabel 내부 텍스트 중앙 정렬 */
    }
    )");
    QHBoxLayout *statusBtnLayout = new QHBoxLayout;
    statusBtnLayout->setAlignment(Qt::AlignCenter);
    statusBtnLayout->addWidget(statusRpi);
    statusBtnLayout->addWidget(statusCam);
    statusBtnLayout->addWidget(statusStm32);
    statusWidget->setLayout(statusBtnLayout);

    QHBoxLayout *leftHeaderHDiv = new QHBoxLayout;
    leftHeaderHDiv->setAlignment(Qt::AlignLeft);
    leftHeaderHDiv->addWidget(titleImgLabel);
    leftHeaderHDiv->addWidget(titleTextLabel);


    QHBoxLayout *statusLayout = new QHBoxLayout;
    statusLayout->setAlignment(Qt::AlignLeft);
    statusLayout->setSpacing(5);
    statusLayout->addWidget(stopSelector);
    statusLayout->addSpacing(20);
    statusLayout->addWidget(statusWidget);

    QVBoxLayout *leftHeader = new QVBoxLayout;
    leftHeader->addLayout(leftHeaderHDiv);
    leftHeader->addLayout(statusLayout);

    settingsButton = new QPushButton("⚙️ Settings");
    settingsButton->setStyleSheet("color: white; background: transparent; font-size: 14px;");
    settingsButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->setAlignment(Qt::AlignTop);
    topLayout->addLayout(leftHeader);
    topLayout->addStretch();
    topLayout->addWidget(settingsButton);

    QWidget *topWidget = new QWidget(this);
    topWidget->setLayout(topLayout);

    // stream frame
    QLabel *streamTitle = new QLabel("📺");
    streamTitle->setStyleSheet("font-size: 18px; color: white;");
    streamTitle->setAlignment(Qt::AlignLeft);

    streamSelector = new QComboBox(this);
    streamSelector->addItem("Live Stream");
    streamSelector->addItem("Recorded Video");
    streamSelector->setStyleSheet("font-size: 13px; background-color: #313131; color: white; padding: 2px 8px;");

    videoWidget = new QVideoWidget(this);
    videoWidget->setFixedSize(800, 450);
    videoWidget->hide();

    mediaPlayer = new QMediaPlayer(this);
    mediaPlayer->setVideoOutput(videoWidget);


    streamArea = new QLabel(this);
    streamArea->setFixedSize(800, 450);
    streamArea->setStyleSheet("background-color: black; border: 2px solid #444; border-radius: 0px;");
    streamArea->setAlignment(Qt::AlignCenter);
    streamArea->setText("<font color='gray'>영상 스트리밍 출력</font>");

    QHBoxLayout *titleLayout = new QHBoxLayout;
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->addSpacing(5);
    titleLayout->addWidget(streamTitle);
    titleLayout->addWidget(streamSelector);
    titleLayout->addStretch();

    // stream 전체 묶는 수직 레이아웃
    QVBoxLayout *streamLayout = new QVBoxLayout;
    streamLayout->setAlignment(Qt::AlignVCenter);
    streamLayout->addLayout(titleLayout);
    streamLayout->addSpacing(5);
    streamLayout->addWidget(streamArea, 0, Qt::AlignHCenter);
    streamLayout->addWidget(videoWidget, 0, Qt::AlignHCenter);

    QFrame *streamFrame = new QFrame(this);
    streamFrame->setFixedSize(830, 510);
    streamFrame->setStyleSheet("background-color: #2a2a2a; border-radius: 20px;");
    streamFrame->setLayout(streamLayout);

    connect(streamSelector, &QComboBox::currentTextChanged, this, [=](const QString &mode){
        if (mode == "Live Stream") {
            videoWidget->hide();
            streamArea->show();

            if (videoThread == nullptr) {
                videoThread = new VideoThread(this);
                connect(videoThread, &VideoThread::frameReady, this, [=](const QImage &img) {
                    qDebug() << "🟡 frameReady 수신됨. 크기:" << img.size();
                    streamArea->setPixmap(QPixmap::fromImage(img).scaled(streamArea->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
                });
                videoThread->start();
            } else if (!videoThread->isRunning()) {
                videoThread->start();
            }

        } else if (mode == "Recorded Video") {
            streamArea->hide();
            videoWidget->show();

            if (videoThread) {
                videoThread->stop();
                videoThread->deleteLater();
                videoThread = nullptr;
            }

            QString videoPath = "http://192.168.0.59/output.mp4";
            mediaPlayer->setSource(QUrl(videoPath));
            mediaPlayer->play();
        }
    });

    // bus frame
    QFrame *busFrame = new QFrame(this);
    busFrame->setFixedSize(385, 510);
    busFrame->setStyleSheet("background-color: #2a2a2a; border-radius: 20px;");

    QLabel *busNumberHeader = new QLabel("Bus Number", busFrame);
    QLabel *platformHeader = new QLabel("Platform", busFrame);
    busNumberHeader->setAlignment(Qt::AlignCenter);
    platformHeader->setAlignment(Qt::AlignCenter);
    busNumberHeader->setFixedSize(160, 40);
    platformHeader->setFixedSize(160, 40);
    QString headerStyle = R"(
        background-color: #000;
        color: white;
        border-radius: 20px;
        font-size: 14px;
        font-weight: bold;
    )";
    busNumberHeader->setStyleSheet(headerStyle);
    platformHeader->setStyleSheet(headerStyle);

    QHBoxLayout *headerLayout = new QHBoxLayout;
    headerLayout->setAlignment(Qt::AlignHCenter);
    headerLayout->addWidget(busNumberHeader);
    headerLayout->addSpacing(20);
    headerLayout->addWidget(platformHeader);

    QVBoxLayout *busFrameLayout = new QVBoxLayout(busFrame);
    busFrameLayout->setAlignment(Qt::AlignTop);

    infoTable = new QTableWidget(4, 2, busFrame);
    infoTable->setFixedSize(320, 360);
    infoTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    infoTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    infoTable->horizontalHeader()->setVisible(false);
    infoTable->verticalHeader()->setVisible(false);
    infoTable->setShowGrid(false);
    infoTable->setStyleSheet(
        "QTableWidget { background-color: transparent; color: white; border: none; }"
        "QTableView::item { border: none; }"
        );
    infoTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    infoTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 2; ++col) {
            QLabel *cell = new QLabel("", infoTable);
            cell->setAlignment(Qt::AlignCenter);
            QString style = "background-color: transparent; color: white;";
            QString platformText = "";
            if (col == 1) {
                switch (row) {
                case 0: platformText = "    P1"; break;
                case 1: platformText = "    P2"; break;
                case 2: platformText = "    P3"; break;
                case 3: platformText = "    P4"; break;
                }
                style += "font-size: 18px; font-weight: bold;";
            }
            cell->setText(platformText);
            cell->setStyleSheet(style);
            infoTable->setCellWidget(row, col, cell);
        }
    }

    busFrameLayout->addLayout(headerLayout);
    busFrameLayout->addSpacing(20);
    busFrameLayout->addWidget(infoTable, 0, Qt::AlignHCenter);
    busFrameLayout->addSpacing(0);

    // 중단 중앙 레이아웃
    QHBoxLayout *middleLayout = new QHBoxLayout;
    middleLayout->addWidget(streamFrame);
    middleLayout->addSpacing(20);
    middleLayout->addWidget(busFrame);

    // 전체 메인 레이아웃
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(topWidget);
    mainLayout->addLayout(middleLayout);
    mainLayout->addSpacing(45);

    QWidget *central = new QWidget(this);
    central->setLayout(mainLayout);
    setCentralWidget(central);
}


void MainWindow::setupConnections() {
    connect(settingsButton, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);
    connect(stopSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onStopChanged);
}

void MainWindow::onSettingsClicked() {
    settingsDlg->exec();
}

void MainWindow::onStopChanged(int index) {
    Q_UNUSED(index)
}


void MainWindow::fetchBusData() {
    qDebug() << " fetchBusData() 진입. 현재 apiUrlBase:" << apiUrlBase;

    /*
    QNetworkRequest request(QUrl("http://192.168.0.76/cgi-bin/sequence.cgi"));
    QNetworkReply *reply = networkManager->get(request);
    */

    QUrl url(apiUrlBase);
    url.setPort(apiPort);  // ✅ 포트 추가!!!
    QNetworkRequest request(url);
    QNetworkReply *reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            qDebug() << "🎥 서버 응답: " << response;

            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);

            if (doc.isNull()) {
                reply->deleteLater();
                return;
            }

            if (!doc.isObject()) {
                reply->deleteLater();
                return;
            }

            QJsonObject rootObj = doc.object();

            // ✅ online 상태값을 UI에 반영
            QJsonArray online = rootObj["online"].toArray();
            if (online.size() == 3) {
                auto updateLabelStatus = [this](QLabel* label, int status) {
                    QString icon;
                    QString color;
                    QString name;

                    if (label == statusRpi) name = "Server";
                    else if (label == statusCam) name = "Camera";
                    else if (label == statusStm32) name = "Display";

                    if (status == 1) {         // ✅ 정상
                        icon = "🟢";
                        color = "limegreen";
                    } else if (status == 2) {  // ✅ 점검필요
                        icon = "🔴";
                        color = "red";
                    } else {                   // ✅ 오프라인
                        icon = "⚫";
                        color = "black";
                    }

                    QString labelText = "<span style='color:white'>" + name + "</span> " +
                                        "<span style='color:" + color + "'>" + icon + "</span>";
                    label->setText(labelText);
                    label->setStyleSheet("background-color: #313131;");
                };

                updateLabelStatus(statusRpi, online[0].toInt());    // 라즈베리파이 상태
                updateLabelStatus(statusCam, online[1].toInt());    // 카메라 상태
                updateLabelStatus(statusStm32, online[2].toInt());  // STM32 상태
            }

            QJsonArray sequence = rootObj["sequence"].toArray();

            for (int row = 0; row < 4; ++row) {
                QLabel *cell = qobject_cast<QLabel *>(infoTable->cellWidget(row, 0));
                if (cell) {
                    cell->setText("");
                }
            }

            for (const QJsonValue &val : sequence) {
                QJsonObject obj = val.toObject();
                int platform = obj["platform"].toInt();
                QString busNum = obj["busNumber"].toString();
                int row = platform - 1;

                if (row >= 0 && row < 4) {
                    QLabel *cell = qobject_cast<QLabel *>(infoTable->cellWidget(row, 0));
                    if (cell) {
                        cell->setText(busNum);
                    }
                }
            }
        }
        reply->deleteLater();
    });
}

void MainWindow::playRecordedVideo() {
    QString videoPath = "http://192.168.0.59/videos/output.mp4"; // 실제 URL로 바꾸기
    mediaPlayer->setSource(QUrl(videoPath));
    mediaPlayer->play();
}


MainWindow::~MainWindow() {
    if (videoThread) {
        videoThread->stop();
        videoThread->deleteLater();
    }
    delete ui;
}
