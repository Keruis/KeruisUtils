#include "FloatingBall.h"

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
      m_hoveredLayer(-1),
      m_ballShrinkProgress(1.0),
      m_expandedLayerCount(0),
      m_eyeOpenProgress(1.0),
      m_jellyRestoreAnimation(nullptr),
      m_dockDirection(DockDirection::None)
{
    setupWindowFlags();
    setVisualStyle();
    centerToScreen();
    updateCenterPosition();
    setupHoverTimer();

    m_layerOpacities.resize(m_layerCount, 1.0);

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

    m_menuRootNodes = TESTgenerateMenu({5, 6, 4, 8}, 0,  "");

    generateMenuLayers();
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
}

// ======= 绘制 =======

void FloatingBall::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    drawTrail(painter);

    switch (m_dockDirection) {
        case DockDirection::Left:
        case DockDirection::Right:
            drawDockedVerticalCapsule(painter);
            return;
        case DockDirection::Top:
        case DockDirection::Bottom:
            drawDockedHorizontalCapsule(painter);
            return;
        case DockDirection::None:
            break;
    }

    if (m_ballShrinkProgress > 0.0) {
        drawBall(painter);
    }

    if (m_showSegments && m_ballShrinkProgress <= 0.0) {
        drawSegments(painter);
    }
}

void FloatingBall::drawBall(QPainter& painter) {
    QPoint center = rect().center();
    double r = m_innerRadius * m_ballShrinkProgress;

    double scaleX = 1.0 - m_jellyOffset.x() / (r * 2.0);
    double scaleY = 1.0 - m_jellyOffset.y() / (r * 2.0);
    scaleX = std::clamp(scaleX, 0.85, 1.15);
    scaleY = std::clamp(scaleY, 0.85, 1.15);

    painter.save();
    painter.translate(center);
    painter.scale(scaleY, scaleX);
    painter.translate(-center);

    QRadialGradient gradient;
    gradient.setCenter(center);
    gradient.setFocalPoint(center.x() - r * 0.3, center.y() - r * 0.3);
    gradient.setRadius(r);

    gradient.setColorAt(0.0, QColor(141,196,253, 200));
    gradient.setColorAt(1.0, QColor(141,196,253, 140));

    QRectF ellipseRect(
        center.x() - r,
        center.y() - r,
        r * 2,
        r * 2
    );

    double middleRadius = r * 0.5;
    double middleInnerRadius = middleRadius * 0.7;

    QRectF middleRect(
        center.x() - middleRadius,
        center.y() - middleRadius,
        middleRadius * 2,
        middleRadius * 2
    );

    QRectF middleInnerRect(
        center.x() - middleInnerRadius,
        center.y() - middleInnerRadius,
        middleInnerRadius * 2,
        middleInnerRadius * 2
    );

    QPainterPath middlePath, middleInnerPath;
    middlePath.addEllipse(middleRect);
    middleInnerPath.addEllipse(middleInnerRect);
    middlePath = middlePath.subtracted(middleInnerPath);

    QColor middleColor = QColor(178,219,251, 200);

    double innerRadius = r * 0.3;
    QRectF innerCircle(
        center.x() - innerRadius,
        center.y() - innerRadius,
        innerRadius * 2,
        innerRadius * 2
    );

    QColor innerColor = QColor(124,164,223, 220);

    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(ellipseRect);
    painter.setBrush(middleColor);
    painter.drawPath(middlePath);
    painter.setBrush(innerColor);
    painter.drawEllipse(innerCircle);

    if (m_selected) {
        QRadialGradient glowGradient;
        glowGradient.setCenter(center);
        glowGradient.setFocalPoint(center);
        glowGradient.setRadius(r * 1.5);

        glowGradient.setColorAt(0.0, QColor(209,248,255, 100));
        glowGradient.setColorAt(0.7, QColor(209,248,255, 30));
        glowGradient.setColorAt(1.0, QColor(209,248,255, 0));

        painter.setBrush(glowGradient);
        painter.setPen(Qt::NoPen);

        QRectF glowRect(
            center.x() - r * 1.5,
            center.y() - r * 1.5,
            r * 3.0,
            r * 3.0
        );

        painter.drawEllipse(glowRect);
    }

    QPainterPath lowerMask, upperMask;

    qreal offset = r * (1.0 - m_eyeOpenProgress);

    QRectF arcRect(center.x() - r, center.y() - r, r * 2, r * 2);
    QPointF lowerEyeCenter(center.x(), center.y() + offset);
    QPointF upperEyeCenter(center.x(), center.y() - offset);

    QPointF startLowerPoint(center.x() - r, center.y());
    QPointF endLowerPoint(center.x() + r, center.y());

    lowerMask.moveTo(startLowerPoint);
    lowerMask.quadTo(lowerEyeCenter, endLowerPoint);
    lowerMask.arcTo(arcRect, 0, -180);

    if (m_selected) {
        gradient.setColorAt(0.0, QColor(220, 220, 220, 255));
        gradient.setColorAt(1.0, QColor(180, 180, 180, 255));
    } else {
        gradient.setColorAt(0.0, QColor(150, 150, 150, 255));
        gradient.setColorAt(1.0, QColor(100, 100, 100, 255));
    }

    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawPath(lowerMask);

    upperMask.moveTo(startLowerPoint);
    upperMask.arcTo(arcRect, 180, -180);
    upperMask.quadTo(upperEyeCenter, startLowerPoint);
    painter.drawPath(upperMask);
    painter.restore();
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

            QColor color;

            if (layer == m_hoveredLayer && i == m_hoveredIndex) {
                color = QColor(255, 0, 0, 180);
            } else if (m_selectedSegments.size() > layer && m_selectedSegments[layer] == i) {
                color = QColor(180, 180, 180, 140);
            } else {
                color = QColor(100, 100, 100, 140);
            }

            int baseAlpha = color.alpha();
            double layerOpacity = (m_layerOpacities.size() > layer) ? m_layerOpacities[layer] : 1.0;
            color.setAlphaF((baseAlpha / 255.0) * layerOpacity);

            QPainterPath path;
            path.moveTo(center);
            path.arcMoveTo(outerRect, angle);
            path.arcTo(outerRect, angle, visibleSpan);
            path.arcTo(innerRect, angle + visibleSpan, -visibleSpan);
            path.closeSubpath();

            painter.setBrush(color);
            painter.setPen(Qt::NoPen);
            painter.drawPath(path);

            double midAngle = angle + visibleSpan / 2.0;
            double textRadius = (radius + innerRect.width() / 2.0) / 2.0;
            double rad = midAngle * M_PI / 180.0;
            QPointF textPos(
                center.x() + textRadius * std::cos(rad),
                center.y() - textRadius * std::sin(rad)
            );

            QColor textColor = Qt::white;
            textColor.setAlphaF(layerOpacity);
            painter.setPen(textColor);
            painter.setFont(QFont("Arial", 10));
            QString text;
            if (layer < m_menuLayers.size() && i < m_menuLayers[layer].size()) {
                text = QString::fromStdString(m_menuLayers[layer][i]);
            } else {
                text = "?";
            }
            QRectF textRect(textPos.x() - 20, textPos.y() - 10, 40, 20);
            painter.drawText(textRect, Qt::AlignCenter, text);

            angle += spanAngle + gapAngle;
        }

        innerRect = outerRect;
        const int spacing = m_layerSpacing[layer];
        innerRect.adjust(-spacing, -spacing, spacing, spacing);
    }
}


void FloatingBall::drawDockedVerticalCapsule(QPainter &painter) {
    QPoint center = rect().center();

    constexpr int capsuleWidth = 20;
    constexpr int capsuleHeight = 60;
    constexpr int radius = capsuleWidth / 2;

    QRectF topArcRect(
        center.x() - radius,
        center.y() - capsuleHeight / 2,
        capsuleWidth,
        capsuleWidth
    );

    QRectF bottomArcRect(
        center.x() - radius,
        center.y() + capsuleHeight / 2 - capsuleWidth,
        capsuleWidth,
        capsuleWidth
    );

    QPainterPath path;

    path.moveTo(bottomArcRect.left(), bottomArcRect.center().y());

    path.arcTo(bottomArcRect, 180, 180);

    path.lineTo(topArcRect.right(), topArcRect.center().y());

    path.arcTo(topArcRect, 0, 180);

    path.lineTo(bottomArcRect.left(), bottomArcRect.center().y());

    path.closeSubpath();

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(220, 220, 220, 200));
    painter.setPen(Qt::NoPen);
    painter.drawPath(path);
}


void FloatingBall::drawDockedHorizontalCapsule(QPainter &painter) {
    QPoint center = rect().center();

    constexpr int capsuleWidth = 60;
    constexpr int capsuleHeight = 20;
    constexpr int radius = capsuleHeight / 2;

    QRectF leftArcRect(
        center.x() - capsuleWidth / 2,
        center.y() - radius,
        capsuleHeight,
        capsuleHeight
    );

    QRectF rightArcRect(
        center.x() + capsuleWidth / 2 - capsuleHeight,
        center.y() - radius,
        capsuleHeight,
        capsuleHeight
    );

    QPainterPath path;

    path.moveTo(leftArcRect.center());

    path.arcTo(leftArcRect, 90, 180);

    path.lineTo(rightArcRect.left(), rightArcRect.bottom());

    path.arcTo(rightArcRect, 270, 180);

    path.lineTo(leftArcRect.right() - radius, leftArcRect.top());

    path.closeSubpath();

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(220, 220, 220, 200));
    painter.setPen(Qt::NoPen);
    painter.drawPath(path);
}


void FloatingBall::drawTrail(QPainter &painter) {
    if (m_trailPoints.size() < 2)
        return;

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    for (int i = 0; i < m_trailPoints.size() - 1; ++i) {
        // 将全局/父坐标转为本地坐标
        QPointF pt1_local = mapFromParent(m_trailPoints[i]);
        QPointF pt2_local = mapFromParent(m_trailPoints[i + 1]);

        // 获取 pt2 到 pt1 的方向向量
        QPointF dir = pt1_local - pt2_local;
        qreal len = std::hypot(dir.x(), dir.y());

        // 归一化反方向单位向量
        QPointF offset(0, 0);
        if (len > 1e-3)
            offset = dir / len * 10.0;

        QPointF pt3 = pt1_local + offset;

        float alpha = 150.0f * (1.0f - float(i) / m_trailPoints.size());
        QColor color(141, 196, 253, int(alpha));
        painter.setBrush(color);

        float radius = m_innerRadius * 0.5 * (1.0f - float(i) / m_trailPoints.size());

        painter.drawEllipse(pt3 - QPointF(radius, radius), radius, radius);
    }

    painter.restore();
}


// ======= 动画 =======

void FloatingBall::transformLayerAnimated(int layer) {
    if (layer >= m_expandedLayerCount) {
        return;
    }

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
    m_ballShrinkProgress = 1.0;

    QVariantAnimation* shrinkAnim = new QVariantAnimation(this);
    shrinkAnim->setDuration(300);
    shrinkAnim->setStartValue(1.0);
    shrinkAnim->setEndValue(0.0);
    shrinkAnim->setEasingCurve(QEasingCurve::InOutCubic);

    connect(shrinkAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
        m_ballShrinkProgress = value.toDouble();
        update();
    });

    connect(shrinkAnim, &QVariantAnimation::finished, this, [this]() {
        m_showSegments = true;
        transformLayerAnimated(0);
    });

    shrinkAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void FloatingBall::onAllAnimationsFinished() {
    if (m_expanded) {
        m_showSegments = false;
        m_drawProgress.clear();
        m_ballShrinkProgress = 1.0;
        update();
    }

    m_expanded = !m_expanded;
}

void FloatingBall::transformToCollapsedState() {
    collapseLayerAnimated(m_expandedLayerCount - 1);
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
    m_showSegments = false;
    m_drawProgress.clear();

    QVariantAnimation* growAnim = new QVariantAnimation(this);
    growAnim->setDuration(300);
    growAnim->setStartValue(0.0);
    growAnim->setEndValue(1.0);
    growAnim->setEasingCurve(QEasingCurve::OutCubic);

    connect(growAnim, &QVariantAnimation::valueChanged, this, [this](const QVariant& value) {
        m_ballShrinkProgress = value.toDouble();
        update();
    });

    connect(growAnim, &QVariantAnimation::finished, this, [this]() {
        m_expanded = false;
        m_expandedLayerCount = 0;
    });

    growAnim->start(QAbstractAnimation::DeleteWhenStopped);
}

void FloatingBall::collapseLayersInRange(int fromLayer, int toLayer) {
    if (fromLayer > toLayer) std::swap(fromLayer, toLayer);
    collapseLayerAnimatedInRange(toLayer, fromLayer);
}

void FloatingBall::collapseLayerAnimatedInRange(int currentLayer, int stopAtLayer) {
    if (currentLayer < stopAtLayer) {
        return;
    }

    double endRadius = (currentLayer == 0)
        ? m_innerRadius
        : m_layerRadii[currentLayer - 1] + m_layerSpacing[currentLayer - 1];

    double startRadius = m_layerRadii[currentLayer];

    QVariantAnimation* animation = new QVariantAnimation(this);
    animation->setDuration(250);
    animation->setStartValue(startRadius);
    animation->setEndValue(endRadius);
    animation->setEasingCurve(QEasingCurve::InCubic);

    connect(animation, &QVariantAnimation::valueChanged, this, [=, this](const QVariant& value) {
        m_currentLayerRadii[currentLayer] = value.toDouble();

        double totalDistance = startRadius - endRadius;
        m_drawProgress[currentLayer] = std::clamp(
            (m_currentLayerRadii[currentLayer] - endRadius) / totalDistance,
            0.0, 1.0
        );

        update();
    });

    connect(animation, &QVariantAnimation::finished, this, [=, this]() {
        if (m_selectedSegments.size() > currentLayer)
            m_selectedSegments[currentLayer] = -1;

        collapseLayerAnimatedInRange(currentLayer - 1, stopAtLayer);
    });

    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void FloatingBall::fadeLayersInRange(int fromLayer, int toLayer) {
    if (fromLayer > toLayer) std::swap(fromLayer, toLayer);
    fadeOutLayerInRange(fromLayer, toLayer);
}

void FloatingBall::fadeOutLayerInRange(int currentLayer, int toLayer) {
    if (currentLayer > toLayer || currentLayer >= m_layerCount) return;

    QVariantAnimation* fadeOut = new QVariantAnimation(this);
    fadeOut->setDuration(200);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);
    fadeOut->setEasingCurve(QEasingCurve::OutQuad);

    connect(fadeOut, &QVariantAnimation::valueChanged, this, [=, this](const QVariant& value) {
        if (m_layerOpacities.size() > currentLayer)
            m_layerOpacities[currentLayer] = value.toDouble();
        update();
    });

    connect(fadeOut, &QVariantAnimation::finished, this, [=, this]() {
        fadeInLayerInRange(currentLayer, toLayer);
    });

    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
}

void FloatingBall::fadeInLayerInRange(int currentLayer, int toLayer) {
    QVariantAnimation* fadeIn = new QVariantAnimation(this);
    fadeIn->setDuration(200);
    fadeIn->setStartValue(0.0);
    fadeIn->setEndValue(1.0);
    fadeIn->setEasingCurve(QEasingCurve::InQuad);

    connect(fadeIn, &QVariantAnimation::valueChanged, this, [=, this](const QVariant& value) {
        if (m_layerOpacities.size() > currentLayer)
            m_layerOpacities[currentLayer] = value.toDouble();
        update();
    });

    connect(fadeIn, &QVariantAnimation::finished, this, [=, this]() {
        fadeOutLayerInRange(currentLayer + 1, toLayer);
    });

    fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
}


// ======= 拖动处理 =======


void FloatingBall::mousePressEvent(QMouseEvent * event) {
    if (event->button() == Qt::LeftButton) {
        startJellyRestoreElastic();
        stickToNearestEdge(true);
        if (m_expandedLayerCount == 0) {
            storeDragOffset(event->globalPos());
            return;
        }

        if (m_expandedLayerCount >= m_layerCount) {
            if ((m_hoveredLayer + 1) == m_layerCount) {
                for (int i = 0; i < m_selectedSegments.size(); ++i) {
                    int index = m_selectedSegments[i];
                    if (index != -1) {
                        emit segmentClicked(i, index);
                    }
                }
                
                collapseLayerAnimatedInRange(m_hoveredLayer + 1, m_expandedLayerCount - 1);
                m_expandedLayerCount = m_hoveredLayer;
                return;
            }
        }

        if (m_hoveredLayer >= 0 && m_hoveredIndex >= 0) {
            if (m_selectedSegments.size() < m_layerCount)
                m_selectedSegments.resize(m_layerCount, -1);
            m_selectedSegments[m_hoveredLayer] = m_hoveredIndex;

            generateMenuLayers();
        }

        if (m_hoveredLayer + 1 < m_layerCount) {
            if ((m_hoveredLayer + 1) == m_expandedLayerCount) {
                m_expandedLayerCount++;
                transformLayerAnimated(m_hoveredLayer + 1);
            } else {
                if (m_hoveredLayer != (m_expandedLayerCount - 2)) {
                    collapseLayersInRange(m_hoveredLayer + 1, m_expandedLayerCount - 1);
                    m_expandedLayerCount = m_hoveredLayer + 1;
                } else {
                    fadeLayersInRange(m_hoveredLayer + 1, m_expandedLayerCount - 1);
                }
            }
        }

    } else if (event->button() == Qt::RightButton) {
        if (m_dockDirection == DockDirection::None) {
            if (!m_expanded && (m_expandedLayerCount == 0)) {
                m_expandedLayerCount = 1;
                m_hoverTimer->start();
                transformToRadialMenu();
            } else {
                m_hoverTimer->stop();
                transformToCollapsedState();
                m_expandedLayerCount = 0;
                std::ranges::fill(m_selectedSegments, -1);
            }
        }
    }
}


void FloatingBall::mouseMoveEvent(QMouseEvent *event) {
    stickToNearestEdge(false);
    if (event->buttons() & Qt::LeftButton) {
        if (m_expandedLayerCount == 0) {
            performDrag(event->globalPos());
        }
    }
}

void FloatingBall::closeEvent(QCloseEvent *event) {
    QWidget::closeEvent(event);
}

void FloatingBall::enterEvent(QEnterEvent* event) {
    m_selected = true;
    QPropertyAnimation* anim = new QPropertyAnimation(this, "eyeOpenProgress");
    anim->setStartValue(m_eyeOpenProgress);
    anim->setEndValue(-0.8);
    anim->setDuration(500);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void FloatingBall::leaveEvent(QEvent* event) {
    m_selected = false;

    if (m_dockDirection != DockDirection::None) {
        stickToNearestEdge(true);
    }

    if (m_jellyOffset.manhattanLength() > 0.1) {
        startJellyRestoreElastic();
    }

    QPropertyAnimation* anim = new QPropertyAnimation(this, "eyeOpenProgress");
    anim->setStartValue(m_eyeOpenProgress);
    anim->setEndValue(1.8);
    anim->setDuration(500);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void FloatingBall::storeDragOffset(const QPoint& globalPos) {
    m_dragOffset = globalPos - frameGeometry().topLeft();
    m_lastDragPos = globalPos;
}

void FloatingBall::performDrag(const QPoint& globalPos) {
    QPointF delta = globalPos - m_lastDragPos;

    m_jellyOffset = delta * 3.5;//* 0.5;
    m_jellyOffset.setX(std::clamp(m_jellyOffset.x(), -100.0, 100.0));
    m_jellyOffset.setY(std::clamp(m_jellyOffset.y(), -100.0, 100.0));

    m_trailPoints.emplace_back(mapToGlobal(rect().center()));
    if (m_trailPoints.size() > 20) {
        m_trailPoints.pop_front();
    }

    m_lastDragPos = globalPos;

    move(globalPos - m_dragOffset);
    updateCenterPosition();
    update();
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

void FloatingBall::startJellyRestoreElastic() {
    if (m_jellyRestoreAnimation) {
        m_jellyRestoreAnimation->stop();
        m_jellyRestoreAnimation->deleteLater();
    }

    m_jellyRestoreAnimation = new QVariantAnimation(this);
    m_jellyRestoreAnimation->setDuration(600);
    m_jellyRestoreAnimation->setStartValue(m_jellyOffset);
    m_jellyRestoreAnimation->setEndValue(QPointF(0, 0));

    m_jellyRestoreAnimation->setEasingCurve(QEasingCurve::OutElastic);

    connect(m_jellyRestoreAnimation, &QVariantAnimation::valueChanged, this, [=](const QVariant &value) {
        m_jellyOffset = value.toPointF();
        update();
    });

    connect(m_jellyRestoreAnimation, &QVariantAnimation::finished, this, [=]() {
        m_jellyOffset = QPointF(0, 0);
        update();
    });

    m_jellyRestoreAnimation->start();
}

void FloatingBall::stickToNearestEdge(bool isDocked) {
    QRect screenRect = QGuiApplication::primaryScreen()->availableGeometry();
    QPoint center = rect().center() + this->pos();

    const int r = static_cast<int>(m_innerRadius);
    const int snapThreshold = 50;

    int leftDist   = std::abs(center.x() - r - screenRect.left());
    int rightDist  = std::abs(center.x() + r - screenRect.right());
    int topDist    = std::abs(center.y() - r - screenRect.top());
    int bottomDist = std::abs(center.y() + r - screenRect.bottom());

    int minDist = std::min({leftDist, rightDist, topDist, bottomDist});
    QPoint targetCenter = center;

    if (minDist > snapThreshold) {
        m_dockDirection = DockDirection::None;
        return;
    }

    if (minDist == leftDist) {
        targetCenter.setX(screenRect.left() + r - 35);
        m_dockDirection = DockDirection::Left;
    } else if (minDist == rightDist) {
        targetCenter.setX(screenRect.right() - r + 35);
        m_dockDirection = DockDirection::Right;
    } else if (minDist == topDist) {
        targetCenter.setY(screenRect.top() + r - 35);
        m_dockDirection = DockDirection::Top;
    } else if (minDist == bottomDist) {
        targetCenter.setY(screenRect.bottom() - r + 35);
        m_dockDirection = DockDirection::Bottom;
    }

    if (isDocked) {
        QPoint offset = rect().center();
        QPoint targetTopLeft = targetCenter - offset;

        QPropertyAnimation* anim = new QPropertyAnimation(this, "pos");
        anim->setDuration(200);
        anim->setStartValue(this->pos());
        anim->setEndValue(targetTopLeft);
        anim->setEasingCurve(QEasingCurve::OutQuad);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
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
    } else {
        m_hoveredLayer = layer;
        m_hoveredIndex = getHoveredSegmentFromAngle(layer, angle);
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

// ======= Menu =======

void FloatingBall::generateMenuLayers() {
    m_menuLayers.clear();

    const std::vector<MenuNode>* currentLevel = &m_menuRootNodes;

    for (int depth = 0; ; ++depth) {
        std::vector<std::string> layerLabels;

        for (const auto& node : *currentLevel) {
            layerLabels.push_back(node.label);
        }
        m_menuLayers.push_back(layerLabels);

        if (depth  >= m_selectedSegments.size()) break;

        int selected = m_selectedSegments[depth];
        if (selected < 0 || selected >= currentLevel->size())
            break;

        currentLevel = &(*currentLevel)[selected].children;
        if (currentLevel->empty())
            break;
    }
}

std::vector<FloatingBall::MenuNode> FloatingBall::TESTgenerateMenu(
    const std::vector<int>& branchingPerLevel,
    int depth,
    const std::string& prefix
) {
    std::vector<MenuNode> nodes;

    if (depth >= branchingPerLevel.size())
        return nodes;

    int branchingFactor = branchingPerLevel[depth];

    for (int i = 0; i < branchingFactor; ++i) {
        std::string label;

        if (prefix.empty()) {
            label = std::string(1, 'A' + i);
        } else if (depth + 1 == branchingPerLevel.size()) {
            label = prefix + std::to_string(i + 1);
        } else if (std::isdigit(prefix.back())) {
            label = prefix + static_cast<char>('a' + i);
        } else {
            label = prefix + std::to_string(i + 1);
        }

        auto children = TESTgenerateMenu(branchingPerLevel, depth + 1, label);
        nodes.emplace_back(label, children);
    }

    return nodes;
}