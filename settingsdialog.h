#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QLocalSocket>         // ✅ 소켓 관련 가장 위로
#define SOCKET_PATH "/tmp/camera_socket"  // ✅ 소켓 경로 정의
#include <QMediaPlayer>    // ✅ 영상 재생용
#include <QVideoWidget>    // ✅ 영상 출력용
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
#include <QSettings>   // ✅ 설정 저장용 추가
#include <QLabel>
#include <QNetworkAccessManager>
#include <QVBoxLayout>
#include <QLocalSocket>
#define SOCKET_PATH "/tmp/camera_socket"

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

    // ✅ MainWindow에서 접근할 수 있도록 getter 함수 추가
    QString getApiUrl() const;
    quint16 getPort() const;
    bool getAutoConnect() const;

    int getBrightness() const;
    int getContrast() const;
    int getExposure() const;
    int getSaturation() const;

    // ✅ MainWindow에서 설정 복원할 수 있도록 setter 함수 추가
    void setApiUrl(const QString &url);
    void setPort(quint16 port);
    void setAutoConnect(bool enabled);

    void setBrightness(int value);
    void setContrast(int value);
    void setExposure(int value);
    void setSaturation(int value);

signals:
    void configUpdated(); // 나중에 MainWindow로 신호 보내기
    void cameraConfigUpdateRequested(int brightness, int contrast, int exposure, int saturation); // ✅ 카메라 설정 서버에 POST 요청

private slots:
    void onUpdateClicked();
    void onCancelClicked();
    void onPageChanged(int index);  // ✅ 사이드바 클릭 시 처리

private:
    // 💡 왼쪽 사이드바
    QListWidget *pageSelector;          // ✅ 누락되었던 변수
    QStackedWidget *stackedPages;       // ✅ 설정 페이지 전환용

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

    // Camera settings
    QLabel *originalFrame;               // 기존 영상 (JPEG 1장)
    QVideoWidget *previewVideo;         // 변경 후 영상 스트리밍용
    QMediaPlayer *previewPlayer;        // 영상 재생기
    QPushButton *applyBtn;

    QNetworkAccessManager *netManager;

    QVBoxLayout *cameraLayoutContainer;
    QLocalSocket *unixSocket;

    // ✅ Cancel 시 복원용 값 저장
    QString originalApiUrl;
    quint16 originalPort;
    bool originalAutoConnect;
    int originalBrightness;
    int originalContrast;
    int originalExposure;
    int originalSaturation;

    void saveOriginalValues();  // ✅ 변경 전 값 저장
    void restoreOriginalValues(); // ✅ Cancel 시 복원
};


#endif // SETTINGSDIALOG_H
