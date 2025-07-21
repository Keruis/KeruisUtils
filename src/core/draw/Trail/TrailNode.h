#ifndef TRAILNODE_H
#define TRAILNODE_H

#include <QPointF>

struct TrailNode {
    QPointF pos;
    float scale;

    TrailNode(const QPointF& pos, float scale) : pos(pos), scale(scale) {}

    static constexpr QPointF lerp(const QPointF& a, const QPointF& b, float t) noexcept {
        return QPointF{
            std::lerp(a.x(), b.x(), t),
            std::lerp(a.y(), b.y(), t)
        };
    }
};

#endif //TRAILNODE_H
