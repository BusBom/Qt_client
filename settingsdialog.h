#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

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

signals:
    void configUpdated(); // 나중에 MainWindow로 신호 보내기

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
};


#endif // SETTINGSDIALOG_H
