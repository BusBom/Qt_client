#include "roi_frame.h"

RoiFrame::RoiFrame(QWidget *parent)
    : QLabel(parent)
{
    setMouseTracking(true);
}

QVector<QVector<QPoint>> RoiFrame::getRois() const
{
    return rois;
}

void RoiFrame::setBackgroundImage(const QPixmap &img) {
    background = img;
    update();  // 화면 다시 그리기
}

void RoiFrame::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint clicked = event->pos();
        currentPoints.append(clicked);

        // 점이 4개 모이면 ROI 하나로 간주
        if (currentPoints.size() == 4) {
            rois.append(currentPoints);
            currentPoints.clear();
            emit roiUpdated();  // SettingsDialog에서 roiPolygons 갱신
        }

        update();  // paintEvent 재호출
    }
}

void RoiFrame::paintEvent(QPaintEvent *event)
{
    QLabel::paintEvent(event);
    QPainter painter(this);
    QPen pen(Qt::yellow);
    pen.setWidth(2);
    painter.setPen(pen);
    painter.drawPixmap(rect(), background);  // 전체 영역에 배경 표시
    // 이후 ROI 선분 그리기 코드는 여기에 이어짐


    // 이미 완료된 ROI들
    for (const QVector<QPoint> &roi : rois) {
        if (roi.size() == 4) {
            for (int i = 0; i < 4; ++i) {
                painter.drawLine(roi[i], roi[(i + 1) % 4]);
            }
        }
    }

    // 아직 진행 중인 점들
    if (!currentPoints.isEmpty()) {
        QPen tempPen(Qt::cyan);
        tempPen.setStyle(Qt::DashLine);
        painter.setPen(tempPen);

        for (int i = 0; i < currentPoints.size() - 1; ++i) {
            painter.drawLine(currentPoints[i], currentPoints[i + 1]);
        }
    }
}

void RoiFrame::clear()
{
    rois.clear();
    currentPoints.clear();
    update();  // 화면 재갱신
}
