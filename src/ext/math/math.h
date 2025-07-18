#pragma once

#include <concepts>
#include <cmath>

#define MATH_ATTR __attribute__((always_inline, hot, const))

namespace Keruis::Math {

    // 通用插值函数
    template <std::floating_point Fp>
    MATH_ATTR constexpr Fp curve(const Fp f, const Fp from, const Fp to) noexcept {
        return (f < from) ? 0.f : ((f > to) ? 1.f : (f - from) / (to - from));
    }

    // 从极坐标转换为自定义类型（如 Vec2）
    template <typename Ty_, std::floating_point Fp>
    requires requires(Fp x, Fp y) { Ty_(x, y); } // ✅ 修复：不再用 std::floating_point auto
    MATH_ATTR constexpr Ty_ from_polar_rad(Fp angleRad, Fp length) noexcept {
        return Ty_(std::cos(angleRad) * length, std::sin(angleRad) * length);
    }

    // 判断两个向量是否在给定距离内相等
    template <typename Ty_, std::floating_point Fp>
    requires (
        requires(const Ty_& t, Fp x, Fp y) {         // ✅ 修复 #1：参数写法
            { t.x() } -> std::convertible_to<Fp>;    // ✅ 修复 #2：使用 convertible_to 而不是 floating_point
            { t.y() } -> std::convertible_to<Fp>;
            { Ty_(x, y) };
        } ||
        requires(const Ty_& t, Fp x, Fp y) {
            { t.x } -> std::convertible_to<Fp>;
            { t.y } -> std::convertible_to<Fp>;
            { Ty_(x, y) };
        }
    )
    MATH_ATTR constexpr bool equals(const Ty_& v, const Ty_& other, Fp dst) noexcept {
        if constexpr (requires { v.x(); v.y(); }) {
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