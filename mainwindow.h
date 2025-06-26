#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTimer>
#include <QNetworkAccessManager>

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

private:
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

    void setupUI();
    void setupConnections();
};

#endif // MAINWINDOW_H
