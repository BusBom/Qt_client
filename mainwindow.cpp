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
    // 🚍 Title + Icon
    QLabel *titleLabel = new QLabel("<img src=:/img/images/bus_face.png width=32 height=32> <b style='font-size:25px;'>Live Dashboard</b>");
    titleLabel->setStyleSheet("color: white;");

    // 🟢 정류장 선택 dropdown
    stopSelector = new QComboBox(this);
    stopSelector->addItems({"래미안아파트.파이낸셜뉴스", "신분당선 강남역", "지하철2호선 강남역", "논현역"});
    stopSelector->setStyleSheet("background-color: #2e2e2e; color: white; border-radius: 30px; padding: 4px;");

    // 🟢 상태 표시
    statusRpi = new QLabel("Raspberry Pi: 🔴");
    statusCam = new QLabel("Camera: 🔴");
    statusStm32 = new QLabel("STM32: 🔴");

    QHBoxLayout *statusLayout = new QHBoxLayout;
    statusLayout->addWidget(stopSelector);
    statusLayout->addSpacing(10);
    statusLayout->addWidget(statusRpi);
    statusLayout->addWidget(statusCam);
    statusLayout->addWidget(statusStm32);

    QVBoxLayout *leftHeader = new QVBoxLayout;
    leftHeader->addWidget(titleLabel);
    leftHeader->addLayout(statusLayout);

    // ⚙️ Settings 버튼
    settingsButton = new QPushButton("⚙️ Settings");
    settingsButton->setStyleSheet("color: white; background: transparent; font-size: 14px;");
    settingsButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addLayout(leftHeader);
    topLayout->addStretch();
    topLayout->addWidget(settingsButton);

    QWidget *topWidget = new QWidget(this);
    topWidget->setLayout(topLayout);

    // 📺 Live Stream 표시 박스 (프레임 포함)
    QLabel *streamTitle = new QLabel("📺 Live Stream");
    streamTitle->setStyleSheet("font-size: 16px; color: white;");

    QLabel *streamArea = new QLabel(this); // 영상 공간 placeholder
    streamArea->setFixedSize(800, 480); // ✅ 영상 영역 사이즈 크게 설정
    streamArea->setStyleSheet("background-color: black; border: 2px solid #444;");
    streamArea->setAlignment(Qt::AlignCenter);
    streamArea->setText("<font color='gray'>영상 스트리밍 출력</font>");

    QVBoxLayout *streamLayout = new QVBoxLayout;
    streamLayout->addWidget(streamTitle);
    streamLayout->addWidget(streamArea);

    // 🚌 버스 테이블
    busTable = new QTableWidget(0, 2, this);
    busTable->setHorizontalHeaderLabels({"Bus Number", "Platform"});
    busTable->horizontalHeader()->setStretchLastSection(false);
    busTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    busTable->setColumnWidth(0, 200);  // ✅ 컬럼 폭 줄이기
    busTable->setColumnWidth(1, 200);
    busTable->setFixedWidth(400);      // ✅ 테이블 전체 폭 제한
    busTable->setFixedHeight(520);
    busTable->setStyleSheet(
        "QTableWidget { background-color: #2a2a2a; color: white; border-radius: 20px; }"
        "QHeaderView::section { background-color: rgba(0, 0, 0, 0.8); color: white; }"
        "QTableWidget::item { background-color: rgba(0, 0, 0, 0.8); }"
        );
    busTable->verticalHeader()->setVisible(false);
    busTable->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    // 전체 가운데 부분 레이아웃
    QHBoxLayout *middleLayout = new QHBoxLayout;
    middleLayout->addLayout(streamLayout);
    middleLayout->addSpacing(20);
    middleLayout->addWidget(busTable);

    // 전체 메인 레이아웃
    QVBoxLayout *mainLayout = new QVBoxLayout;
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

