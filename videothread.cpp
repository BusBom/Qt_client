#include "videothread.h"
#include "configmanager.h"

#include <QDebug>
#include <opencv2/opencv.hpp>

VideoThread::VideoThread(QObject *parent)
    : QThread(parent), stopped(false) {}

void VideoThread::run() {
    qDebug() << "?? VideoThread run() started";

    cv::VideoCapture cap(ConfigManager::getValue("rtsp_url").toStdString(), cv::CAP_FFMPEG);
    if (!cap.isOpened()) {
        qWarning() << "❌ VideoCapture open 실패";
        return;
    }

    qDebug() << "✅ VideoCapture 열기 성공";
    qDebug() << "Width:" << cap.get(cv::CAP_PROP_FRAME_WIDTH);
    qDebug() << "Height:" << cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    qDebug() << "FPS:" << cap.get(cv::CAP_PROP_FPS);

    cv::Mat frame;
    int count = 0;
    int maxRetries = 100;

    while (!stopped.loadRelaxed()) {
        bool success = cap.read(frame);

        if (!success || frame.empty()) {
            qDebug() << "⚠️ frame 읽기 실패 or 비어 있음. count=" << count;
            QThread::msleep(100);
            count++;
            if (count > maxRetries) break;
            continue;
        }

        qDebug() << "✅ frame 수신됨";

        cv::Mat rgb;
        cv::cvtColor(frame, rgb, cv::COLOR_BGR2RGB);
        QImage img((const uchar *)rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888);
        emit frameReady(img.copy());

        QThread::msleep(33);
    }

    cap.release();
    qDebug() << "🎞 VideoThread 종료됨";
}

void VideoThread::stop() {
    stopped.storeRelease(true);
    wait();
}
