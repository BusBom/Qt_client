#ifndef ROI_FRAME_H
#define ROI_FRAME_H

#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QVector>
#include <QPoint>
#include <QPen>

class RoiFrame : public QLabel
{
    Q_OBJECT

public:
    explicit RoiFrame(QWidget *parent = nullptr);

    void setBackgroundImage(const QPixmap &img);

    // 선택된 ROI들 반환
    QVector<QVector<QPoint>> getRois() const;
    void clear();

signals:
    void roiUpdated();  // ROI가 추가될 때마다 emit

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<QVector<QPoint>> rois;     // 전체 ROI 목록
    QVector<QPoint> currentPoints;     // 현재 ROI 구성 중인 점들
    QPixmap background;
};

#endif // ROI_FRAME_H
