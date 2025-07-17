#ifndef TRAILWIDGET_H
#define TRAILWIDGET_H

#include <QWidget>
#include "trailpath.h"

#include <QWidget>
#include <QMouseEvent>
#include <QTimer>
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
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        trail.draw(painter, 15.0f, 1.0f, QColor(255, 0, 0, 160));
    }

    void mouseMoveEvent(QMouseEvent* event) override {
        trail.addPoint(event->position());
        update();
    }

private:
    TrailPath trail{100};
};

#endif //TRAILWIDGET_H
