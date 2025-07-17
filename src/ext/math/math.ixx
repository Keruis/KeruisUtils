export module math;

#include <concepts>
#include <cmath>

#define MATH_ATTR __attribute__((always_inline,hot,const))

namespace Keruis::Math {
    export template <std::floating_point Fp>
    MATH_ATTR constexpr Fp curve(const Fp f, const Fp from, const Fp to) noexcept {
        return (f < from) ? 0.f : ((f > to) ? 1.f : (f - from) / (to - from));
    }

    export template <typename Ty_, std::floating_point Fp>
    requires requires(std::floating_point auto x, std::floating_point auto y)
    {{Ty_(x, y)};}
    MATH_ATTR constexpr Ty_ from_polar_rad(Fp angleRad, Fp length) noexcept {
        return Ty_(std::cos(angleRad) * length, std::sin(angleRad) * length);
    }

    export template <typename Ty_, std::floating_point Fp>
    requires (
        requires(const Ty_& t, std::floating_point auto x, std::floating_point auto y)
    {
        {t.x()} -> std::floating_point; {t.y()} -> std::floating_point;
        {Ty_(x, y)};
    } ||
        requires(const Ty_& t, std::floating_point auto x, std::floating_point auto y)
    {
        {t.x} -> std::floating_point; {t.y} -> std::floating_point;
        {Ty_(x, y)};
    }
    )
    MATH_ATTR constexpr bool equals(const Ty_& v, const Ty_& other, Fp dst) noexcept {
        if constexpr (requires {v.x(); v.y();}) {
            const Fp dx = other.x() - v.x();
            const Fp dy = other.y() - v.y();
            return (dx * dx + dy * dy) < dst * dst;
        } else {
            const Fp dx = other.x - v.x;
            const Fp dy = other.y - v.y;
            return (dx * dx + dy * dy) < dst * dst;
        }
    }
}