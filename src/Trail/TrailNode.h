#ifndef TRAILNODE_H
#define TRAILNODE_H

#include <QPointF>

struct TrailNode {
    QPointF pos;
    float scale = 1.0f;

    TrailNode() = default;
    TrailNode(const QPointF& p, float s = 1.0f) : pos(p), scale(s) {}

    constexpr QPointF lerp(const QPointF& a, const QPointF& b, float t) noexcept {
        return QPointF{
            std::lerp(a.x(), b.x(), t),
            std::lerp(a.y(), b.y(), t)
        };
    }
};

#endif //TRAILNODE_H
