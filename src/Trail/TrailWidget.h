#ifndef TRAILWIDGET_H
#define TRAILWIDGET_H

#include <QWidget>
#include "trailpath.h"

#include <QWidget>
#include <QMouseEvent>
#include <QTimer>
#include <QPainter>
#include <QImage>
#include <QColor>
#include <QPolygonF>
#include <QPointF>

#include "TrailPath.h"

class TrailWidget : public QWidget {
    Q_OBJECT
public:
    explicit TrailWidget(QWidget* parent = nullptr)
        : QWidget(parent) {
        setAttribute(Qt::WA_OpaquePaintEvent);
        setAttribute(Qt::WA_NoSystemBackground);
        setMouseTracking(true);
        setAutoFillBackground(false);
    }

protected:
    void paintEvent(QPaintEvent*) override {
        qDebug() << "paintEvent called!";
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        trail.each(5.0f, [&](int index, const QPointF& p1, const QPointF& p2, const QPointF& p3, const QPointF& p4, float progress1, float progress2) {
            painter.setBrush(QColor(255, 0, 0, 255 * progress1));

            QPolygonF quad({p1, p2, p3, p4});
            painter.drawPolygon(quad);
        });
    }

    void mouseMoveEvent(QMouseEvent* event) override {
        trail.addPoint(event->pos(), 1.0f);
        update();
    }

private:
    TrailPath trail;
};

#endif //TRAILWIDGET_H
