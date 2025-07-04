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

#include "settingsdialog.h"

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
    void updateConnectionStatus();
    void fetchBusData();
    void playRecordedVideo();

private:
    const QString PATH = QCoreApplication::applicationDirPath();
    Ui::MainWindow *ui;

    SettingsDialog *settingsDlg;
    QComboBox *stopSelector;
    QLabel *statusRpi;
    QLabel *statusCam;
    QLabel *statusStm32;
    QPushButton *settingsButton;
    QTableWidget *busTable;
    QTimer *pollTimer;
    QNetworkAccessManager *networkManager;

    // ✅ [추가] 실시간 버스 정보 출력 테이블
    QTableWidget *infoTable;

    // ✅ [추가] 버스 정보 갱신 타이머
    QTimer *busTimer;

    void setupUI();
    void setupConnections();

    QMediaPlayer *mediaPlayer;
    QVideoWidget *videoWidget;
    QComboBox *streamSelector;
};

#endif // MAINWINDOW_H
