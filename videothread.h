#ifndef VIDEOTHREAD_H
#define VIDEOTHREAD_H

#include <QThread>
#include <QDebug>
#include <QImage>
#include <QAtomicInteger>
#include <opencv2/opencv.hpp>

class VideoThread : public QThread {
    Q_OBJECT

public:
    explicit VideoThread(QObject *parent = nullptr);
    void run() override;
    void stop();

signals:
    void frameReady(const QImage &image);

private:
    QAtomicInteger<bool> stopped;
};

#endif // VIDEOTHREAD_H
