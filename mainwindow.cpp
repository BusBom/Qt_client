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
    this->setFixedSize(1240, 650);
    setStyleSheet("background-color: #1e1e1e; color: #ccc;");

    QIcon icon(PATH + "/images/bus_face.png");
    setWindowIcon(icon);

    setupUI();
    setupConnections();

    settingsDlg = new SettingsDialog(this);
    networkManager = new QNetworkAccessManager(this);

    // SettingsDialog 시그널 연결
    connect(settingsDlg, &SettingsDialog::configUpdated, this, [=]() {
        // 네트워크 설정 반영
        apiUrlBase = settingsDlg->getApiUrl();
        apiPort = settingsDlg->getPort();
        autoConnect = settingsDlg->getAutoConnect();
    });

    connect(settingsDlg, &SettingsDialog::cameraConfigUpdateRequested, this, [=](int brightness, int contrast, int exposure, int saturation) {
        // 카메라 설정 서버 전송
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
            qDebug() << "📷 카메라 설정 응답:" << response;

            QJsonDocument doc = QJsonDocument::fromJson(response);
            if (!doc.isNull() && doc.isObject()) {
                QJsonObject resCam = doc.object().value("camera").toObject();
                settingsDlg->setBrightness(resCam.value("brightness").toInt());
                settingsDlg->setContrast(resCam.value("contrast").toInt());
                settingsDlg->setExposure(resCam.value("exposure").toInt());
                settingsDlg->setSaturation(resCam.value("saturation").toInt());

                qDebug() << "✅ 설정 반영됨 -> 밝기:" << resCam.value("brightness").toInt()
                         << ", 명암:" << resCam.value("contrast").toInt()
                         << ", 노출:" << resCam.value("exposure").toInt()
                         << ", 채도:" << resCam.value("saturation").toInt();
            }
            reply->deleteLater();
        });
    });

    // 버스 정보 실시간 갱신용 타이머
    busTimer = new QTimer(this);
    connect(busTimer, &QTimer::timeout, this, &MainWindow::fetchBusData);
    busTimer->start(1000);

    apiUrlBase = "http://192.168.0.59/cgi-bin/sequence.cgi";
    apiPort = 80;
    autoConnect = false;
}

void MainWindow::setupUI() {
    // 🚍 Title + Icon
    QLabel *titleImgLabel = new QLabel("<img src='" + PATH + "/images/bus_face.png' width=32 height=32>");
    titleImgLabel->setContentsMargins(3, 0, 0, 0);  // 왼쪽에서 오른쪽으로 3px 이동
    QLabel *titleTextLabel = new QLabel("<b style='font-size:22px; color: white;'> Live Dashboard</b>");
    titleTextLabel->setContentsMargins(0, 0, 0, 2);
    QFont hanwhaFont("Hanwha L", 13);
    titleTextLabel->setFont(hanwhaFont);

    stopSelector = new QComboBox(this);
    stopSelector->setMinimumContentsLength(22);
    stopSelector->setFixedHeight(35);
    stopSelector->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    stopSelector->setStyleSheet(R"(
    QComboBox {
        background-color: #2c2c2c;
        border: 0px solid #444;
        color: #ccc;
        padding: 4px 8px;
    }
    QComboBox QAbstractItemView {
        background-color: #1e1e1e;
        color: #ccc;
        selection-background-color: #444;
        border: none;
        outline: none;
        box-shadow: none;
    }
)");

    stopSelector->addItems({"래미안아파트.파이낸셜뉴스", "신분당선 강남역", "지하철2호선 강남역", "논현역"});

    statusRpi = new QLabel("Server: 🔴");
    statusRpi->setStyleSheet("background-color: #313131;");
    statusCam = new QLabel("Camera: 🔴");
    statusCam->setStyleSheet("background-color: #313131;");
    statusStm32 = new QLabel("Display: 🔴");
    statusStm32->setStyleSheet("background-color: #313131;");

    statusRpi->setFixedHeight(20);   // 더 작게도 가능 (예: 18, 16)
    statusCam->setFixedHeight(20);
    statusStm32->setFixedHeight(20);

    QWidget *statusWidget = new QWidget;
    statusWidget->setObjectName("statusWidget");
    statusWidget->setStyleSheet(R"(
    #statusWidget {
        background-color: #313131;
        border: 1px solid #313131;
        padding-left: 20px;
        padding-right: 20px;

    }

    QLabel {
        qproperty-alignment: 'AlignCenter';
        font-size: 11px;
    }
    )");

    QHBoxLayout *statusBtnLayout = new QHBoxLayout;
    statusBtnLayout->setAlignment(Qt::AlignCenter);
    statusBtnLayout->addWidget(statusRpi);
    statusBtnLayout->addWidget(statusCam);
    statusBtnLayout->addWidget(statusStm32);
    statusWidget->setLayout(statusBtnLayout);
    statusWidget->setFixedHeight(35);
    statusBtnLayout->setContentsMargins(13, 0, 8, 0);
    statusBtnLayout->setSpacing(12);

    QHBoxLayout *leftHeaderHDiv = new QHBoxLayout;
    leftHeaderHDiv->setAlignment(Qt::AlignLeft);
    leftHeaderHDiv->setContentsMargins(5, 0, 0, 0);
    leftHeaderHDiv->setSpacing(10);
    leftHeaderHDiv->addWidget(titleImgLabel);
    leftHeaderHDiv->addWidget(titleTextLabel);


    QHBoxLayout *statusLayout = new QHBoxLayout;
    statusLayout->setAlignment(Qt::AlignLeft);
    statusLayout->setContentsMargins(11, 0, 0, 0);
    statusLayout->addWidget(stopSelector);
    statusLayout->addSpacing(20);
    statusLayout->addWidget(statusWidget);

    QVBoxLayout *leftHeader = new QVBoxLayout;
    leftHeader->addLayout(leftHeaderHDiv);
    leftHeader->addLayout(statusLayout);

    // ⚙ Settings 버튼
    settingsButton = new QPushButton("⚙ Settings");
    settingsButton->setStyleSheet(R"(
        background: transparent;
        color: lightgray;
        font-weight: normal;
    )");
    settingsButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QVBoxLayout *settingsLayout = new QVBoxLayout;
    settingsLayout->setContentsMargins(0, 10, 15, 0);
    settingsLayout->setSpacing(0);
    settingsLayout->addWidget(settingsButton, 0, Qt::AlignTop | Qt::AlignRight);

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->setContentsMargins(0, 2, 0, 0);
    topLayout->setSpacing(0);
    topLayout->addLayout(leftHeader);
    topLayout->addStretch();
    topLayout->addLayout(settingsLayout);

    QWidget *topWidget = new QWidget(this);
    topWidget->setLayout(topLayout);


    // stream frame
    QLabel *streamTitle = new QLabel("📺");
    streamTitle->setStyleSheet("font-size: 18px; color: white;");
    streamTitle->setAlignment(Qt::AlignLeft);

    streamSelector = new QComboBox(this);
    streamSelector->addItem("Live Stream");
    streamSelector->addItem("Recorded Video");
    streamSelector->setStyleSheet(R"(
       QComboBox {
           background-color: #2c2c2c;
           border: 0px solid #444;
           color: #ccc;
           padding: 4px 8px;
           font-size: 13px;
       }
       QComboBox QAbstractItemView {
           background-color: #1e1e1e;
           color: #ccc;
           selection-background-color: #444;
           border: none;
           outline: none;
       }
    )");


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
    streamLayout->addSpacing(2);
    streamLayout->addWidget(streamArea, 0, Qt::AlignHCenter);
    streamLayout->addWidget(videoWidget, 0, Qt::AlignHCenter);

    QFrame *streamFrame = new QFrame(this);
    streamFrame->setFixedSize(830, 500);
    streamFrame->setStyleSheet("background-color: #2a2a2a; border-radius: 0px;");
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

    // 🚍 Bus Info Frame
    QFrame *busFrame = new QFrame(this);
    busFrame->setFixedSize(350, 500);
    busFrame->setStyleSheet("background-color: #2a2a2a; border: 1px solid #2a2a2a; border-radius: 0px;");

    // ✅ Header Frame
    QFrame *headerFrame = new QFrame;
    headerFrame->setFixedHeight(40);
    headerFrame->setStyleSheet("background-color: #1c1c1c; border-bottom: 1px solid #333;");

    QLabel *busNumberLabel = new QLabel("Bus Number");
    QLabel *platformLabel = new QLabel("Platform");

    QFont headerFont("Pretendard", 10);
    busNumberLabel->setFont(headerFont);
    platformLabel->setFont(headerFont);
    busNumberLabel->setStyleSheet("color: #bbb;");
    platformLabel->setStyleSheet("color: #bbb;");
    busNumberLabel->setAlignment(Qt::AlignCenter);
    platformLabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout *headerLayout = new QHBoxLayout(headerFrame);
    headerLayout->setContentsMargins(0, 0, 0, 0);
    headerLayout->setSpacing(0);
    headerLayout->addWidget(busNumberLabel);
    headerLayout->addWidget(platformLabel);

    // ✅ Info Table (헤더 제외 460px)
    infoTable = new QTableWidget(4, 2, busFrame);
    infoTable->setFixedHeight(460);
    infoTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    infoTable->setMinimumWidth(busFrame->width());
    infoTable->setStyleSheet(R"(
        QTableWidget {
            background-color: transparent;
            color: #ddd;
            border: none;
            font-size: 13px;
        }
        QTableView::item {
            border-bottom: 1px solid #333;
            border-right: 1px solid #333;
            border-left: 1px solid #333;
            padding: 0px;  /* ✅ 간격 제거 */
        }
    )");

    infoTable->horizontalHeader()->setVisible(false);
    infoTable->verticalHeader()->setVisible(false);
    infoTable->setShowGrid(false);
    infoTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    infoTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    infoTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    infoTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // ✅ Layout
    QVBoxLayout *busFrameLayout = new QVBoxLayout(busFrame);
    busFrameLayout->setContentsMargins(0, 0, 0, 0);
    busFrameLayout->setSpacing(0);
    busFrameLayout->addWidget(headerFrame);
    busFrameLayout->addWidget(infoTable, 1);

    // ✅ Cell 내용 설정
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 2; ++col) {
            QLabel *cell = new QLabel("", infoTable);
            cell->setAlignment(Qt::AlignCenter);
            cell->setContentsMargins(0, 0, 0, 0);  // ✅ 내부 여백 제거
            QString style;

            if (col == 1) {
                switch (row) {
                case 0: cell->setText("P1"); break;
                case 1: cell->setText("P2"); break;
                case 2: cell->setText("P3"); break;
                case 3: cell->setText("P4"); break;
                }
                style = R"(
                background-color: transparent;
                color: white;
                font-size: 16px;
                font-weight: bold;
            )";
            } else {
                style = R"(
                background-color: transparent;
                color: #ccc;
                font-size: 13px;
            )";
            }

            cell->setStyleSheet(style);
            infoTable->setCellWidget(row, col, cell);
        }
    }


    // 중단 중앙 레이아웃
    QHBoxLayout *middleLayout = new QHBoxLayout;
    middleLayout->addWidget(streamFrame);
    middleLayout->addSpacing(1);
    middleLayout->addWidget(busFrame);

    // 전체 메인 레이아웃
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(topWidget);
    mainLayout->addSpacing(5);
    mainLayout->addLayout(middleLayout);

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
    url.setPort(apiPort);
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

            // online 상태값을 UI에 반영
            QJsonArray online = rootObj["online"].toArray();
            if (online.size() == 3) {
                auto updateLabelStatus = [this](QLabel* label, int status) {
                    QString icon;
                    QString color;
                    QString name;

                    if (label == statusRpi) name = "Server";
                    else if (label == statusCam) name = "Camera";
                    else if (label == statusStm32) name = "Display";

                    if (status == 1) {         // 정상
                        icon = "🟢";
                        color = "limegreen";
                    } else if (status == 2) {  // 점검필요
                        icon = "🔴";
                        color = "red";
                    } else {                   // 오프라인
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
    QString videoPath = "http://192.168.0.59/videos/output.mp4"; // 실제 URL
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
