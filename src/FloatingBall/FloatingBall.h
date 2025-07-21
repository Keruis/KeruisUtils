#pragma once

#include <vector>
#include <deque>
#include <ranges>
#include <algorithm>

#include <QWidget>
#include <QPoint>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QApplication>
#include <QPropertyAnimation>
#include <QTimer>
#include <QDateTime>
#include <QGraphicsDropShadowEffect>
#include <QElapsedTimer>

#include "FloatingBall.h"
#include "../core/draw/Trail/TrailPath.h"
#include "../../Script/ClassRegistry.h"
#include "../../Tool/window/WindowController.h"

class FloatingBall : public QWidget {
    Q_OBJECT

    struct MenuNode {
        std::string label;
        std::vector<MenuNode> children;

        MenuNode(const std::string& label, const std::vector<MenuNode>& children) : label(label), children(children) {}
    };

    struct Ripple {
        float progress = 0.0f;
        QDateTime createdAt = QDateTime::currentDateTime();
        QPoint center;
    };

    enum class DockDirection {
        None,
        Left,
        Right,
        Top,
        Bottom
    };

public:
    explicit FloatingBall               (QWidget* parent = nullptr)                                     ;
    ~FloatingBall                       () override                                                     ;

    void setSelected                    (bool selected)            { m_selected = selected; update(); } ;

    [[nodiscard]] QPoint centerGlobalPos()                          const { return m_centerGlobalPos; } ;
    [[nodiscard]] bool   isSelected     ()                          const { return        m_selected; } ;

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
    void drawDockedVerticalCapsule      (QPainter& painter)                                             ;
    void drawDockedHorizontalCapsule    (QPainter& painter)                                             ;
    void drawTrail                      (QPainter& painter)                                             ;
    void storeDragOffset                (const QPoint& globalPos)                                       ;
    void performDrag                    (const QPoint& globalPos)                                       ;

    void transformLayerAnimated         (int layer)                                                     ;
    void transformToRadialMenu          ()                                                              ;
    void onAllAnimationsFinished        ()                                                              ;
    void transformToCollapsedState      ()                                                              ;
    void collapseLayerAnimated          (int layer)                                                     ;
    void onCollapseFinished             ()                                                              ;
    void collapseLayersInRange          (int fromLayer, int toLayer)                                    ;
    void collapseLayerAnimatedInRange   (int currentLayer, int stopAtLayer)                             ;
    void fadeLayersInRange              (int fromLayer, int toLayer)                                    ;
    void fadeOutLayerInRange            (int currentLayer, int toLayer)                                 ;
    void fadeInLayerInRange             (int currentLayer, int toLayer)                                 ;

    void startHoverTimer                ()                                                              ;
    void stopHoverTimer                 ()                                                              ;

    void startJellyRestoreElastic       ()                                                              ;
    void stickToNearestEdge             (bool isDocked)                                                 ;

    void updateHoveredByDirection       ()                                                              ;
    int  getHoveredSegmentFromAngle     (int layer, double angle)     const                             ;

    Q_PROPERTY  (float eyeOpenProgress READ eyeOpenProgress WRITE setEyeOpenProgress)
    float eyeOpenProgress               () const        { return m_eyeOpenProgress;                     }
    void  setEyeOpenProgress            (float value)   {m_eyeOpenProgress = value;update();            }

    void generateMenuLayers             ()                                                              ;
    std::vector<MenuNode> TESTgenerateMenu(
        const std::vector<int>& branchingPerLevel,
        int depth,
        const std::string& prefix
    );

signals:
    void segmentClicked                 (int layer, int index)                                          ;

private:
    std::vector<MenuNode>                    m_menuRootNodes;
    std::vector<std::vector<std::string>>       m_menuLayers;

    double                              m_ballShrinkProgress;

    QTimer*                                     m_hoverTimer;

    bool                                          m_expanded;
    bool                                          m_selected;

    QPoint                                      m_dragOffset;
    QPoint                                 m_centerGlobalPos;
    bool                                        m_isDragging;

    std::vector<qreal>                        m_drawProgress;

    int                                       m_hoveredLayer;
    int                                       m_hoveredIndex;
    int                                   m_lastHoveredLayer;
    std::vector<int>                      m_selectedSegments;
    bool                                      m_showSegments;

    int                                         m_layerCount;
    std::vector<double>                       m_layerSpacing;

    std::vector<double>                         m_layerRadii;
    std::vector<double>                  m_currentLayerRadii;
    std::vector<int>                    m_layerSegmentCounts;
    int                                       m_currentLayer;
    int                                 m_expandedLayerCount;

    std::vector<double>                     m_layerOpacities;

    std::vector<int>                              m_gapAngle;
    double                                     m_innerRadius;

    float                                  m_eyeOpenProgress;

    QPointF                                    m_jellyOffset;
    QPointF                                    m_lastDragPos;
    QVariantAnimation*               m_jellyRestoreAnimation;

    DockDirection                            m_dockDirection;

    TrailPath                                        m_trail;
};