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
      m_layerCount(2),
      m_layerSpacing(20.0),
      m_expanded(false),
      m_selected(false),
      m_drawProgress(0.0),
      m_hoveredIndex(-1),
      m_gapAngle(5),
      m_segmentCount(5),
      m_showSegments(false),
      m_outerRadius(50.0),
      m_innerRadius(40.0)
{
    setupWindowFlags();
    setVisualStyle();
    centerToScreen();
    updateCenterPosition();
    setupHoverTimer();
}

FloatingBall::~FloatingBall() = default;

void FloatingBall::setupWindowFlags() {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
}

void FloatingBall::setVisualStyle() {
    setFixedSize(260, 260);
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
    double r = m_outerRadius;

    QRectF ellipseRect(
        center.x() - r,
        center.y() - r,
        r * 2,
        r * 2
    );

    painter.drawEllipse(ellipseRect);
}

void FloatingBall::drawSegments(QPainter& painter) {
    const int spanAngle = (360 / m_segmentCount) - m_gapAngle;
    int angle = 0;

    const QPoint center = rect().center();

    QRectF outerRect(
        center.x() - m_outerRadius,
        center.y() - m_outerRadius,
        m_outerRadius * 2,
        m_outerRadius * 2
    );

    QRectF innerRect(
        center.x() - m_innerRadius,
        center.y() - m_innerRadius,
        m_innerRadius * 2,
        m_innerRadius * 2
    );

    for (int i = 0; i < m_segmentCount; ++i) {
        int visibleSpan = static_cast<int>(spanAngle * m_drawProgress);
        if (visibleSpan <= 0) continue;

        QColor color = (i == m_hoveredIndex)
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

        angle += spanAngle + m_gapAngle;
    }
}

void FloatingBall::transformToRadialMenu() {
    const double startRadius = m_outerRadius;
    const double targetRadius = m_expanded ? 50.0 : 120.0;

    m_showSegments = true;

    QVariantAnimation* anim = new QVariantAnimation(this);
    anim->setDuration(800);
    anim->setStartValue(startRadius);
    anim->setEndValue(targetRadius);
    anim->setEasingCurve(QEasingCurve::OutCubic);

    connect(anim, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
        m_outerRadius = value.toDouble();
        m_drawProgress = std::clamp((m_outerRadius - 50.0) / (120.0 - 50.0), 0.0, 1.0);
        update();
    });

    connect(anim, &QVariantAnimation::finished, this, [this]() {
        if (m_expanded) {
            m_showSegments = false;
            m_drawProgress = 0.0;
            update();
        }

        m_expanded = !m_expanded;
    });

    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

// ======= 拖动处理 =======

void FloatingBall::mousePressEvent(QMouseEvent* event) {
    if (m_hoveredIndex != -1 && m_expanded) {
        emit segmentClicked(m_hoveredIndex);
        transformToRadialMenu();
    }
    if (event->button() == Qt::LeftButton) {
        storeDragOffset(event->globalPos());
    } else if (event->button() == Qt::RightButton) {
        transformToRadialMenu();
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

    if (delta.manhattanLength() < 5) {
        m_hoveredIndex = -1;
        update();
        return;
    }

    double angle = std::atan2(-delta.y(), delta.x()) * 180 / M_PI;
    if (angle < 0) angle += 360;

    int index = getHoveredSegmentFromAngle(angle);
    if (index != m_hoveredIndex) {
        m_hoveredIndex = index;
        update();
    }
}


int FloatingBall::getHoveredSegmentFromAngle(double angle) const {
    const int totalSegments = m_segmentCount;
    const int spanAngle = (360 / totalSegments) - m_gapAngle;
    const int fullSpan = spanAngle + m_gapAngle;

    for (int i = 0; i < totalSegments; ++i) {
        int startAngle = i * fullSpan;
        int endAngle = startAngle + spanAngle;

        if (startAngle <= angle && angle < endAngle)
            return i;
        else if (endAngle > 360 && angle < (endAngle - 360))
            return i;
    }

    return -1;
}