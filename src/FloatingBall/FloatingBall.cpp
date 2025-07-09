#include "FloatingBall.h"
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QPropertyAnimation>
#include <QScreen>

// ======= 构造 & 初始化 =======

FloatingBall::FloatingBall(QWidget* parent)
    : QWidget(parent),
      m_hoverTimer(new QTimer(this)),
      m_layerCount(4),
      m_expanded(false),
      m_selected(false),
      m_showSegments(false),
      m_innerRadius(40.0),
      m_hoveredIndex(-1),
      m_hoveredLayer(-1)
{
    setupWindowFlags();
    setVisualStyle();
    centerToScreen();
    updateCenterPosition();
    setupHoverTimer();

    m_currentLayer = 0;

    m_layerSpacing.push_back(10.0);
    m_layerSpacing.push_back(10.0);
    m_layerSpacing.push_back(10.0);

    m_layerRadii.push_back(100.0);
    m_layerSegmentCounts.push_back(5);
    m_gapAngle.push_back(5);

    m_layerRadii.push_back(200.0);
    m_layerSegmentCounts.push_back(6);
    m_gapAngle.push_back(5);

    m_layerRadii.push_back(300.0);
    m_layerSegmentCounts.push_back(4);
    m_gapAngle.push_back(5);

    m_layerRadii.push_back(350.0);
    m_layerSegmentCounts.push_back(8);
    m_gapAngle.push_back(5);

    m_currentLayerRadii = m_layerRadii;
}

FloatingBall::~FloatingBall() = default;

void FloatingBall::setupWindowFlags() {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
}

void FloatingBall::setVisualStyle() {
    setFixedSize(760, 760);
}

void FloatingBall::centerToScreen() {
    QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
    move(screenGeometry.center() - rect().center());
}

void FloatingBall::updateCenterPosition() {
    m_centerGlobalPos = geometry().center();
}

void FloatingBall::setupHoverTimer() {
    m_hoverTimer->setInterval(16);
    connect(m_hoverTimer, &QTimer::timeout, this, &FloatingBall::updateHoveredByDirection);
    startHoverTimer();
}

// ======= 绘制 =======

void FloatingBall::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_showSegments) {
        drawSegments(painter);
    } else {
        drawBall(painter);
    }
}

void FloatingBall::drawBall(QPainter& painter) {
    QColor color = m_selected 
                   ? QColor(180, 180, 180, 140)
                   : QColor(100, 100, 100, 140);

    painter.setBrush(color);
    painter.setPen(Qt::NoPen);

    QPoint center = rect().center();
    double r = m_innerRadius;

    QRectF ellipseRect(
        center.x() - r,
        center.y() - r,
        r * 2,
        r * 2
    );

    painter.drawEllipse(ellipseRect);
}

void FloatingBall::drawSegments(QPainter& painter) {
    QPoint center = rect().center();

    QRectF innerRect(
        center.x() - m_innerRadius,
        center.y() - m_innerRadius,
        m_innerRadius * 2,
        m_innerRadius * 2
    );

    for (int layer = 0; layer <= m_layerCount - 1; ++layer) {
        const int segmentCount = m_layerSegmentCounts[layer];
        const double radius = m_currentLayerRadii[layer];
        const int gapAngle = m_gapAngle[layer];
        const int spanAngle = (360 / segmentCount) - gapAngle;

        QRectF outerRect(
            center.x() - radius,
            center.y() - radius,
            radius * 2,
            radius * 2
        );

        int angle = 0;
        m_drawProgress.push_back(0.0);

        for (int i = 0; i < segmentCount; ++i) {
            int visibleSpan = static_cast<int>(spanAngle * m_drawProgress[layer]);
            if (visibleSpan <= 0) continue;

            QColor color = (layer <= m_hoveredLayer && m_selectedSegments.size() > layer && m_selectedSegments[layer] == i)
                ? QColor(180, 180, 180, 140)
                : QColor(100, 100, 100, 140);

            QPainterPath path;
            path.moveTo(center);
            path.arcMoveTo(outerRect, angle);
            path.arcTo(outerRect, angle, visibleSpan);
            path.arcTo(innerRect, angle + visibleSpan, -visibleSpan);
            path.closeSubpath();

            painter.setBrush(color);
            painter.setPen(Qt::NoPen);
            painter.drawPath(path);

            angle += spanAngle + gapAngle;
        }

        innerRect = outerRect;
        const int spacing = m_layerSpacing[layer];
        innerRect.adjust(-spacing, -spacing, spacing, spacing);
    }
}

void FloatingBall::transformLayerAnimated(int layer) {
    if (layer >= m_layerCount) {
        onAllAnimationsFinished();
        return;
    }

    double startRadius = (layer == 0) ? m_innerRadius
                                      : m_layerRadii[layer - 1] + m_layerSpacing[layer - 1];
    double targetRadius = m_layerRadii[layer];

    QVariantAnimation* animation = new QVariantAnimation(this);
    animation->setDuration(350);
    animation->setStartValue(startRadius);
    animation->setEndValue(targetRadius);
    animation->setEasingCurve(QEasingCurve::OutCubic);

    connect(animation, &QVariantAnimation::valueChanged, this, [=, this] (const QVariant& value) {
        m_currentLayerRadii[layer] = value.toDouble();
        m_drawProgress[layer] = std::clamp((m_currentLayerRadii[layer] - startRadius) / (targetRadius - startRadius), 0.0, 1.0);
        update();
    });

    connect(animation, &QVariantAnimation::finished, this, [=, this]() {
        transformLayerAnimated(layer + 1);
    });

    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void FloatingBall::transformToRadialMenu() {
    m_showSegments = true;
    transformLayerAnimated(0);
}

void FloatingBall::onAllAnimationsFinished() {
    if (m_expanded) {
        m_showSegments = false;
        m_drawProgress.clear();
        update();
    }

    m_expanded = !m_expanded;
}

void FloatingBall::transformToCollapsedState() {
    collapseLayerAnimated(m_layerCount - 1);
}

void FloatingBall::collapseLayerAnimated(int layer) {
    if (layer < 0) {
        onCollapseFinished();
        return;
    }

    double endRadius;
    if (layer == 0) {
        endRadius = m_innerRadius;
    } else {
        endRadius = m_layerRadii[layer - 1] + m_layerSpacing[layer - 1];
    }

    double startRadius = m_layerRadii[layer];

    QVariantAnimation* animation = new QVariantAnimation(this);
    animation->setDuration(350);
    animation->setStartValue(startRadius);
    animation->setEndValue(endRadius);
    animation->setEasingCurve(QEasingCurve::InCubic);

    connect(animation, &QVariantAnimation::valueChanged, this, [=, this](const QVariant& value) {
        m_currentLayerRadii[layer] = value.toDouble();

        double totalDistance = startRadius - endRadius;
        m_drawProgress[layer] = std::clamp(
            (m_currentLayerRadii[layer] - endRadius) / totalDistance,
            0.0, 1.0
        );

        update();
    });

    connect(animation, &QVariantAnimation::finished, this, [=, this]() {
        collapseLayerAnimated(layer - 1);
    });

    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void FloatingBall::onCollapseFinished() {
    m_expanded = false;
    m_showSegments = false;
    m_drawProgress.clear();
    update();
}

// ======= 拖动处理 =======

void FloatingBall::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        if (m_expanded) {
            for (int i = 0; i < m_selectedSegments.size(); ++i) {
                int index = m_selectedSegments[i];
                if (index != -1) {
                    emit segmentClicked(i, index);
                }
            }
            transformToCollapsedState();
        } else {
            storeDragOffset(event->globalPos());
        }
    } else if (event->button() == Qt::RightButton) {
        if (!m_expanded) {
            transformToRadialMenu();
        } else {
            transformToCollapsedState();
        }
    }
}

void FloatingBall::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::LeftButton) {
        performDrag(event->globalPos());
    }
}

void FloatingBall::closeEvent(QCloseEvent *event) {
    QWidget::closeEvent(event);
}

void FloatingBall::enterEvent(QEnterEvent* event) {
    m_selected = true;
}

void FloatingBall::leaveEvent(QEvent* event) {
    m_selected = false;
}

void FloatingBall::storeDragOffset(const QPoint& globalPos) {
    m_dragOffset = globalPos - frameGeometry().topLeft();
}

void FloatingBall::performDrag(const QPoint& globalPos) {
    move(globalPos - m_dragOffset);
    updateCenterPosition();
}

void FloatingBall::startHoverTimer() {
    if (!m_hoverTimer->isActive()) {
        m_hoverTimer->start();
    }
}

void FloatingBall::stopHoverTimer() {
    if (m_hoverTimer->isActive()) {
        m_hoverTimer->stop();
    }
}

void FloatingBall::updateHoveredByDirection() {
    QPoint globalMousePos = QCursor::pos();
    QPoint globalCenter = mapToGlobal(rect().center());
    QPointF delta = globalMousePos - globalCenter;
    double distance = std::hypot(delta.x(), delta.y());

    if (distance < 5) {
        m_hoveredLayer = -1;
        m_hoveredIndex = -1;
        update();
        return;
    }

    double angle = std::atan2(-delta.y(), delta.x()) * 180 / M_PI;
    if (angle < 0) angle += 360;

    int layer = -1;
    for (int i = 0; i < m_layerCount; ++i) {
        double innerRadius = (i == 0) ? m_innerRadius : m_currentLayerRadii[i - 1] + m_layerSpacing[i - 1];
        double outerRadius = m_currentLayerRadii[i];
        if (distance >= innerRadius && distance <= outerRadius) {
            layer = i;
            break;
        }
    }

    if (layer == -1) {
        m_hoveredLayer = -1;
        m_hoveredIndex = -1;
        update();
        return;
    }

    m_hoveredLayer = layer;
    m_hoveredIndex = getHoveredSegmentFromAngle(layer, angle);

    if (m_selectedSegments.size() < m_layerCount)
        m_selectedSegments.resize(m_layerCount, -1);

    m_selectedSegments[layer] = m_hoveredIndex;

    for (int i = 0; i < layer; ++i) {
        if (m_selectedSegments[i] == -1) {
            m_selectedSegments[i] = getHoveredSegmentFromAngle(i, angle);
        }
    }

    update();
}


int FloatingBall::getHoveredSegmentFromAngle(int layer, double angle) const {
    const int segmentCount = m_layerSegmentCounts[layer];
    const int gapAngle = m_gapAngle[layer];
    const int spanAngle = (360 / segmentCount) - gapAngle;
    const int fullSpan = spanAngle + gapAngle;

    for (int i = 0; i < segmentCount; ++i) {
        int startAngle = i * fullSpan;
        int endAngle = startAngle + spanAngle;

        if (startAngle <= angle && angle < endAngle)
            return i;
        if (endAngle > 360 && angle < (endAngle - 360))
            return i;
    }

    return -1;
}