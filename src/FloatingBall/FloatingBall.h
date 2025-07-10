#pragma once

#include <vector>
#include <ranges>
#include <algorithm>

#include <QWidget>
#include <QPoint>
#include "../RedialMenu/RedialMenu.h"

class FloatingBall : public QWidget {
    Q_OBJECT

    struct MenuNode {
        std::string label;
        std::vector<MenuNode> children;

        MenuNode(std::string label, std::vector<MenuNode> children) : label(label), children(children) {}
    };

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

    void transformLayerAnimated(int layer);
    void transformToRadialMenu          ()                                                              ;
    void onAllAnimationsFinished();
    void transformToCollapsedState();
    void collapseLayerAnimated(int layer);
    void onCollapseFinished();

    void startHoverTimer                ()                                                              ;
    void stopHoverTimer                 ()                                                              ;

    void updateHoveredByDirection       ()                                                              ;
    int  getHoveredSegmentFromAngle     (int layer, double angle)     const                             ;

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

    std::vector<int>                              m_gapAngle;
    double                                     m_innerRadius;
};