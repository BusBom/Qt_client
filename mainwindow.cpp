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
#include <QJsonDocument>  // ✅ 추가
#include <QJsonObject>    // ✅ 추가
#include <QJsonArray>     // ✅ 추가
#include <QNetworkReply>  // ✅ 추가

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
    pollTimer = new QTimer(this);
    connect(pollTimer, &QTimer::timeout, this, &MainWindow::updateConnectionStatus);
    pollTimer->start(3000);

    // ✅ 버스 정보 실시간 갱신용 타이머
    busTimer = new QTimer(this);
    connect(busTimer, &QTimer::timeout, this, &MainWindow::fetchBusData);
    busTimer->start(1000);
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


    QLabel *streamArea = new QLabel(this);
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

    // 수정됨: 드롭다운에서 Recorded Video 선택 시 자동 재생
    connect(streamSelector, &QComboBox::currentTextChanged, this, [=](const QString &mode){
        if (mode == "Live Stream") {
            videoWidget->hide();
            streamArea->show();
        } else if (mode == "Recorded Video") {
            streamArea->hide();
            videoWidget->show();

            // 자동 재생
            QString videoPath = "http://192.168.0.49/videos/output.mp4";  // 라즈베리파이 주소
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
                case 0: platformText = "    P4"; break;
                case 1: platformText = "    P3"; break;
                case 2: platformText = "    P2"; break;
                case 3: platformText = "    P1"; break;
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

    // ⬅ 중단 중앙 레이아웃
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

void MainWindow::updateConnectionStatus() {
    statusRpi->setText("Server: 🟢");
    statusCam->setText("Camera: 🟢");
    statusStm32->setText("Display: 🟢");
}

void MainWindow::fetchBusData() {
    QNetworkRequest request(QUrl("http://192.168.0.76/cgi-bin/sequence.cgi"));
    QNetworkReply *reply = networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
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
                int row = 4 - platform;

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
    QString videoPath = "http://192.168.0.49/videos/output.mp4"; // 실제 URL로 바꾸기
    mediaPlayer->setSource(QUrl(videoPath));
    mediaPlayer->play();
}


MainWindow::~MainWindow() {
    delete ui;
}
