#ifndef TRAILPATH_H
#define TRAILPATH_H

#include <vector>
#include <algorithm>

#include <QPointF>
#include <QPainter>
#include <QMutex>

#include "TrailNode.h"
#include "../../../ext/math/math.h"

class TrailPath {
public:

    [[nodiscard]] auto size() -> std::size_t {return m_points.size();}

    void clear() {m_points.clear();}

    void addPoint(const QPointF& pos, float scale = 1.0f) {
        if (m_points.size() >= m_capacity) {
            m_points.erase(m_points.begin());
        }
        m_points.emplace_back(TrailNode{pos, scale});
    }

    template <typename Func>
    void each(const float radius, Func consumer, float percent = 1.0f) {
        if (m_points.empty()) return;

        percent = std::clamp(percent, 0.0f, 1.0f);

        float lastAngle = 0.0f;
        const float capSize = static_cast<float>(this->size() - 1) * percent;

        auto drawImpl = [&](
            std::size_t index,
            const TrailNode& prev,
            const TrailNode& next,
            const float prevProg,
            const float nextProg
        ) -> float {
            const QPointF dst = next.pos - prev.pos;
            const float scl = Keruis::Math::curve(static_cast<float>(std::sqrt(dst.x() * dst.x() + dst.y() * dst.y())), 0.0f, 0.5f) * radius * prev.scale / capSize;
            const float z2 = static_cast<float>(std::atan2(-dst.y(), -dst.x()));

            const float z1 = lastAngle;

            const QPointF c = Keruis::Math::from_polar_rad<QPointF>(static_cast<float>(M_PI_2 - z1), scl * prevProg);
            const QPointF n = Keruis::Math::from_polar_rad<QPointF>(static_cast<float>(M_PI_2 - z2), scl * nextProg);

            if (Keruis::Math::equals(c, {}, 0.01f) && Keruis::Math::equals(n, {}, 0.01f)) {
                return z2;
            }

            if constexpr(std::invocable<Func, int, QPointF, QPointF, QPointF, QPointF, float, float>){
                float progressFormer = prevProg / capSize;
                float progressLatter = nextProg / capSize;

                consumer(index, prev.pos - c, prev.pos + c, next.pos + n, next.pos - n, progressFormer, progressLatter);
            } else if constexpr(std::invocable<Func, int, QPointF, QPointF, QPointF, QPointF>){
                consumer(index, prev.pos - c, prev.pos + c, next.pos + n, next.pos - n);
            } else if constexpr(std::invocable<Func, QPointF, QPointF, QPointF, QPointF, float, float>){
                float progressFormer = prevProg / capSize;
                float progressLatter = nextProg / capSize;

                consumer(prev.pos - c, prev.pos + c, next.pos + n, next.pos - n, progressFormer, progressLatter);
            } else if constexpr(std::invocable<Func, QPointF, QPointF, QPointF, QPointF>){
                consumer(prev.pos - c, prev.pos + c, next.pos + n, next.pos - n);
            } else{
                qDebug() << "consumer not supported";
            }

            return z2;
        };

        const float pos = (1 - percent) * size();
        const float ceil = std::floor(pos) + 1;
        const auto initial = static_cast<std::size_t>(ceil);
        const auto initialFloor = initial - 1;
        const auto prog = ceil - pos;
        std::size_t index = initial;

        if (index >= m_points.size()) return;

        TrailNode nodeNext = m_points[index];
        TrailNode nodeCurrent{nodeCurrent.lerp(nodeNext.pos, m_points[initialFloor].pos, prog), std::lerp(nodeNext.scale, m_points[initialFloor].scale, prog)};

        lastAngle = drawImpl(0, nodeCurrent, nodeNext, 0, prog);

        for (; index < m_points.size() - 1; ++index) {
            nodeCurrent = m_points[index];
            nodeNext = m_points[index + 1];

            if (nodeCurrent.scale <= 0.001f && nodeNext.scale <= 0.001f) continue;

            const float cur = index - initial + prog;
            lastAngle = drawImpl(index - initialFloor, nodeCurrent, nodeNext, cur, cur + 1.0f);
        }
    }

private:
    std::vector<TrailNode> m_points;
    std::size_t m_capacity = 100;
};

#endif //TRAILPATH_H
