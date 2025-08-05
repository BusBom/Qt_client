#pragma once

#include <QThread>
#include <QImage>
#include <QMutex>
#include <QWaitCondition>
#include <QString>
#include <atomic>

#include <opencv2/opencv.hpp>

class VideoPreviewThread : public QThread
{
    Q_OBJECT

public:
    explicit VideoPreviewThread(QObject *parent = nullptr);
    ~VideoPreviewThread();

    void setRtspUrl(const QString &url);
    void stop();

signals:
    void frameReady(const QImage &frame);  // 프레임 수신 시 전달

protected:
    void run() override;

private:
    QString rtspUrl;
    std::atomic<bool> running;
};
