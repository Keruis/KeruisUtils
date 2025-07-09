#pragma once

#include <QWidget>
#include <QPoint>
#include "../RedialMenu/RedialMenu.h"

class FloatingBall : public QWidget {
    Q_OBJECT

public:
    explicit FloatingBall               (QWidget* parent = nullptr)                                     ;
    ~FloatingBall                       ()                                                              ;

    void setSelected                    (bool selected)            { m_selected = selected; update(); } ;

    QPoint centerGlobalPos              ()                          const { return m_centerGlobalPos; } ;
    bool   isSelected                   ()                          const { return        m_selected; } ;

protected:
    void paintEvent                     (QPaintEvent*)              override                            ;
    void mousePressEvent                (QMouseEvent*)              override                            ;
    void mouseMoveEvent                 (QMouseEvent*)              override                            ;
    void closeEvent                     (QCloseEvent*)              override                            ;
    void enterEvent                     (QEnterEvent*)              override                            ;
    void leaveEvent                     (QEvent*)                   override                            ;

private:
    void setupWindowFlags               ()                                                              ;
    void setVisualStyle                 ()                                                              ;
    void centerToScreen                 ()                                                              ;
    void updateCenterPosition           ()                                                              ;
    void setupHoverTimer                ()                                                              ;

    void drawBall                       (QPainter& painter)                                             ;
    void drawSegments                   (QPainter& painter)                                             ;
    void storeDragOffset                (const QPoint& globalPos)                                       ;
    void performDrag                    (const QPoint& globalPos)                                       ;

    void transformToRadialMenu          ()                                                              ;

    void startHoverTimer                ()                                                              ;
    void stopHoverTimer                 ()                                                              ;

    void updateHoveredByDirection       ()                                                              ;
    int  getHoveredSegmentFromAngle     (double angle)               const                              ;

signals:
    void segmentClicked                 (int index)                                                     ;

private:
    QTimer*             m_hoverTimer;

    bool                  m_expanded;
    bool                  m_selected;

    QPoint              m_dragOffset;
    QPoint         m_centerGlobalPos;

    qreal             m_drawProgress;

    int               m_hoveredIndex;
    int               m_segmentCount;
    bool              m_showSegments;

    int                 m_layerCount;
    double            m_layerSpacing;

    int                   m_gapAngle;
    double             m_outerRadius;
    double             m_innerRadius;
};