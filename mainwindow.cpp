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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(1280, 720);
    setStyleSheet("background-color: #1e1e1e; color: white;");

    setupUI();
    setupConnections();

    settingsDlg = new SettingsDialog(this);
    networkManager = new QNetworkAccessManager(this);
    pollTimer = new QTimer(this);
    connect(pollTimer, &QTimer::timeout, this, &MainWindow::updateConnectionStatus);
    pollTimer->start(3000);
}

void MainWindow::setupUI() {
    QLabel *titleLabel = new QLabel("<img src=:/img/images/bus_face.png width=32 height=32> <b style='font-size:25px;'>Live Dashboard</b>");
    titleLabel->setStyleSheet("color: white;");
    titleLabel->setAlignment(Qt::AlignLeft);

    stopSelector = new QComboBox(this);
    stopSelector->addItems({"래미안아파트.파이낸셜뉴스", "신분당선 강남역", "지하철2호선 강남역", "논현역"});
    stopSelector->setStyleSheet("background-color: #2e2e2e; color: white; border-radius: 30px; padding: 4px;");

    statusRpi = new QLabel("Raspberry Pi: 🔴");
    statusCam = new QLabel("Camera: 🔴");
    statusStm32 = new QLabel("STM32: 🔴");

    statusRpi->setFixedWidth(140);
    statusCam->setFixedWidth(110);
    statusStm32->setFixedWidth(100);

    QHBoxLayout *statusLayout = new QHBoxLayout;
    statusLayout->setAlignment(Qt::AlignLeft);
    statusLayout->setSpacing(5);
    statusLayout->addWidget(stopSelector);
    statusLayout->addSpacing(10);
    statusLayout->addWidget(statusRpi);
    statusLayout->addWidget(statusCam);
    statusLayout->addWidget(statusStm32);

    QVBoxLayout *leftHeader = new QVBoxLayout;
    leftHeader->addWidget(titleLabel);
    leftHeader->addLayout(statusLayout);

    settingsButton = new QPushButton("⚙️ Settings");
    settingsButton->setStyleSheet("color: white; background: transparent; font-size: 14px;");
    settingsButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QVBoxLayout *settingsWrapper = new QVBoxLayout;
    {
        QHBoxLayout *innerLayout = new QHBoxLayout;
        innerLayout->addStretch();
        innerLayout->addWidget(settingsButton);
        settingsWrapper->addLayout(innerLayout);
    }
    settingsWrapper->addSpacing(5);
    QWidget *settingsWidget = new QWidget(this);
    settingsWidget->setLayout(settingsWrapper);

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->setAlignment(Qt::AlignTop);
    topLayout->addLayout(leftHeader);
    topLayout->addStretch();
    topLayout->addWidget(settingsButton);

    QWidget *topWidget = new QWidget(this);
    topWidget->setLayout(topLayout);

    QLabel *streamTitle = new QLabel("📺 Live Stream");
    streamTitle->setStyleSheet("font-size: 20px; color: white;");

    QLabel *streamArea = new QLabel(this);
    streamArea->setFixedSize(800, 480);
    streamArea->setStyleSheet("background-color: black; border: 2px solid #444;");
    streamArea->setAlignment(Qt::AlignCenter);
    streamArea->setText("<font color='gray'>영상 스트리밍 출력</font>");

    QVBoxLayout *streamLayout = new QVBoxLayout;
    streamLayout->addWidget(streamTitle);
    streamLayout->addWidget(streamArea);

    // ✅ bus frame
    QFrame *busFrame = new QFrame(this);
    busFrame->setFixedSize(385, 510);
    busFrame->setStyleSheet(
        "background-color: #2a2a2a;"
        "border-radius: 20px;"
        );

    QLabel *busNumberHeader = new QLabel("Bus Number", busFrame);
    QLabel *platformHeader = new QLabel("Platform", busFrame);

    busNumberHeader->setAlignment(Qt::AlignCenter);
    platformHeader->setAlignment(Qt::AlignCenter);

    busNumberHeader->setFixedSize(160, 50);
    platformHeader->setFixedSize(160, 50);

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

    QTableWidget *infoTable = new QTableWidget(4, 2, busFrame);
    infoTable->setFixedSize(320, 360);

    infoTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    infoTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    infoTable->horizontalHeader()->setVisible(false);
    infoTable->verticalHeader()->setVisible(false);

    infoTable->setShowGrid(false);
    infoTable->setStyleSheet(
        "QTableWidget {"
        "  background-color: transparent;"
        "  color: white;"
        "  border: none;"
        "}"
        "QTableView::item {"
        "  border: none;"
        "}"
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

            cell->setText(platformText);   // ✅ 글씨 넣는 핵심 코드!!!
            cell->setStyleSheet(style);
            infoTable->setCellWidget(row, col, cell);
        }
    }

    busFrameLayout->addLayout(headerLayout);
    busFrameLayout->addSpacing(10);
    busFrameLayout->addWidget(infoTable, 0, Qt::AlignHCenter);

    QHBoxLayout *middleLayout = new QHBoxLayout;
    middleLayout->addLayout(streamLayout);
    middleLayout->addSpacing(20);
    middleLayout->addWidget(busFrame);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(settingsWidget);
    mainLayout->addWidget(topWidget);
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

void MainWindow::updateConnectionStatus() {
    statusRpi->setText("Raspberry Pi: 🟢");
    statusCam->setText("Camera: 🟢");
    statusStm32->setText("STM32: 🟢");
}

void MainWindow::fetchBusData() {
    // TODO: API 통신 구현 예정
}

MainWindow::~MainWindow() {
    delete ui;
}
