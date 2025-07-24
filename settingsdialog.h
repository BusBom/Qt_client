#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QLocalSocket>
#define SOCKET_PATH "/tmp/camera_socket"

#include <QMediaPlayer>
#include <QVideoWidget>
#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QSlider>
#include <QTimeEdit>
#include <QPushButton>
#include <QStackedWidget>
#include <QListWidget>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QStyleOptionSlider>
#include <QPainter>
#include <QSettings>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QVBoxLayout>
#include <QVector>
#include <QPoint>

// ROI 설정용 커스텀 위젯
#include "roi_frame.h"

class ClickableSlider : public QSlider
{
    Q_OBJECT
public:
    explicit ClickableSlider(Qt::Orientation orientation, QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
};

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);

    // Getter
    QString getApiUrl() const;
    quint16 getPort() const;
    bool getAutoConnect() const;

    int getBrightness() const;
    int getContrast() const;
    int getExposure() const;
    int getSaturation() const;

    // Setter
    void setApiUrl(const QString &url);
    void setPort(quint16 port);
    void setAutoConnect(bool enabled);

    void setBrightness(int value);
    void setContrast(int value);
    void setExposure(int value);
    void setSaturation(int value);

    void updateRoiDisplay();

signals:
    void configUpdated();
    void cameraConfigUpdateRequested(int brightness, int contrast, int exposure, int saturation);

private slots:
    void onUpdateClicked();
    void onCancelClicked();
    void onPageChanged(int index);

private:
    // 사이드바
    QListWidget *pageSelector;
    QStackedWidget *stackedPages;

    // Network
    QLineEdit *apiUrlEdit;
    QLineEdit *portEdit;
    QCheckBox *autoConnectCheck;

    // Camera
    ClickableSlider *brightnessSlider;
    ClickableSlider *contrastSlider;
    ClickableSlider *exposureSlider;
    ClickableSlider *saturationSlider;

    // Sleep Mode
    QTimeEdit *sleepStartEdit;
    QTimeEdit *sleepEndEdit;

    QPushButton *updateBtn;
    QPushButton *cancelBtn;

    QLabel *originalFrame;
    QVideoWidget *previewVideo;
    QMediaPlayer *previewPlayer;
    QPushButton *applyBtn;

    QNetworkAccessManager *netManager;
    QVBoxLayout *cameraLayoutContainer;

    // ROI 설정
    RoiFrame *roiCanvas;
    QLabel *platformCountLabel;
    QVector<QLabel*> coordLabels;
    QVector<QVector<QPoint>> roiPolygons;

    // Cancel 복원용
    QString originalApiUrl;
    quint16 originalPort;
    bool originalAutoConnect;
    int originalBrightness;
    int originalContrast;
    int originalExposure;
    int originalSaturation;

    void saveOriginalValues();
    void restoreOriginalValues();
};

#endif // SETTINGSDIALOG_H
