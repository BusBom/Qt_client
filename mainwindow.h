#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <opencv2/opencv.hpp>

#include "settingsdialog.h"
#include "videothread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSettingsClicked();
    void onStopChanged(int index);
    void fetchBusData();
    void playRecordedVideo();

private:
    const QString PATH = QCoreApplication::applicationDirPath();
    Ui::MainWindow *ui;
    QSslConfiguration createSslConfig();

    SettingsDialog *settingsDlg;
    QComboBox *stopSelector;
    QLabel *statusRpi;
    QLabel *statusCam;
    QLabel *statusStm32;
    QPushButton *settingsButton;
    QTableWidget *busTable;
    QTimer *pollTimer;
    QNetworkAccessManager *networkManager;

    // 실시간 버스 정보 출력 테이블
    QTableWidget *infoTable;

    // 버스 정보 갱신 타이머
    QTimer *busTimer;

    // Stream 관련
    QMediaPlayer *mediaPlayer;
    QVideoWidget *videoWidget;
    QComboBox *streamSelector;
    QLabel *streamArea;

    VideoThread *videoThread = nullptr;

    void setupUI();
    void setupConnections();
    void startLiveStream();

    // 네트워크 설정 저장용 변수
    QString apiUrlBase;
    quint16 apiPort;
    bool autoConnect;
};

#endif // MAINWINDOW_H
