#include "videopreviewthread.h"
#include <QDebug>
#include <QImage>
#include <QThread>

VideoPreviewThread::VideoPreviewThread(QObject *parent)
    : QThread(parent), running(false)
{
}

VideoPreviewThread::~VideoPreviewThread()
{
    stop();
    wait();  // 스레드가 완전히 종료될 때까지 대기
}

void VideoPreviewThread::setRtspUrl(const QString &url)
{
    rtspUrl = url;
}

void VideoPreviewThread::stop()
{
    running = false;
}

void VideoPreviewThread::run()
{
    if (rtspUrl.isEmpty()) {
        qWarning() << "❌ RTSP URL이 설정되지 않았습니다.";
        return;
    }

    running = true;

    cv::VideoCapture cap(rtspUrl.toStdString(), cv::CAP_FFMPEG);

    if (!cap.isOpened()) {
        qWarning() << "❌ RTSP 스트림 열기 실패:" << rtspUrl;
        return;
    }

    qDebug() << "📡 RTSP 스트림 수신 시작:" << rtspUrl;

    cv::Mat frame;
    while (running) {
        if (!cap.read(frame)) {
            qWarning() << "⚠️ 프레임 읽기 실패";
            QThread::msleep(100);
            continue;
        }

        if (frame.empty())
            continue;

        // BGR → RGB 변환
        cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);

        // OpenCV Mat → QImage
        QImage img(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888);

        // QImage 복사본을 emit (frame 메모리 공유 방지)
        emit frameReady(img.copy());

        QThread::msleep(33);  // 약 30 FPS
    }

    cap.release();
    qDebug() << "🛑 VideoPreviewThread 종료됨";
}
