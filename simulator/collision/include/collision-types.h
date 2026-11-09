//------------------------------------------------------------------------------
//
//      Collision detection engine for RRS (based on Jolt Physics)
//      Collision layers and basic types
//
//------------------------------------------------------------------------------

#ifndef     COLLISION_TYPES_H
#define     COLLISION_TYPES_H

#include    <cstdint>

namespace collision
{

/// Слои коллизий мира симулятора.
/// Значения используются как Jolt ObjectLayer (по умолчанию 16 бит)
enum class Layer : std::uint16_t
{
    Terrain         = 0,    ///< Рельеф, земля
    Rail            = 1,    ///< Рельсы (ходовые поверхности)
    Track           = 2,    ///< Элементы пути (шпалы, балласт, стрелки)
    Infrastructure  = 3,    ///< Здания, платформы, опоры, мосты
    ContactNetwork  = 4,    ///< Контактная сеть
    Decoration      = 5,    ///< Декорации (с ПС не контактируют)
    Vegetation      = 6,    ///< Растительность
    Train           = 7,    ///< Кузова и рамы подвижного состава
    Bogie           = 8,    ///< Тележки
    Wheel           = 9,    ///< Колёсные пары
    RoadVehicle     = 10,   ///< Автотранспорт (переезды)
    Player          = 11,   ///< Игрок в пешем режиме

    Count                   ///< Число слоёв (служебное значение)
};

constexpr std::uint16_t layer_count = static_cast<std::uint16_t>(Layer::Count);

constexpr std::uint16_t to_index(Layer layer)
{
    return static_cast<std::uint16_t>(layer);
}

/// Вектор для обмена данными с движком (метры / м/с)
struct Vec3f
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    constexpr Vec3f() = default;
    constexpr Vec3f(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
};

/// Кватернион поворота (единичный по умолчанию)
struct Quatf
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;

    constexpr Quatf() = default;
    constexpr Quatf(float x_, float y_, float z_, float w_)
        : x(x_), y(y_), z(z_), w(w_) {}
};

inline Vec3f operator+(const Vec3f& a, const Vec3f& b)
{
    return Vec3f(a.x + b.x, a.y + b.y, a.z + b.z);
}

inline float dot(const Vec3f& a, const Vec3f& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline Vec3f cross(const Vec3f& a, const Vec3f& b)
{
    return Vec3f(a.y * b.z - a.z * b.y,
                 a.z * b.x - a.x * b.z,
                 a.x * b.y - a.y * b.x);
}

/// Произведение Гамильтона: результат - поворот b, затем поворот a
inline Quatf operator*(const Quatf& a, const Quatf& b)
{
    return Quatf(a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
                 a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
                 a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
                 a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z);
}

inline Vec3f operator*(float s, const Vec3f& v)
{
    return Vec3f(s * v.x, s * v.y, s * v.z);
}

/// Поворот вектора кватернионом (q должен быть единичным)
inline Vec3f rotate(const Quatf& q, const Vec3f& v)
{
    // v' = v + 2 * (q.w * t + cross(u, t)), где t = cross(u, v)
    const Vec3f u(q.x, q.y, q.z);
    const Vec3f t = cross(u, v);
    const Vec3f tt(q.w * t.x + (u.y * t.z - u.z * t.y),
                   q.w * t.y + (u.z * t.x - u.x * t.z),
                   q.w * t.z + (u.x * t.y - u.y * t.x));
    return v + 2.0f * tt;
}

} // namespace collision

#endif // COLLISION_TYPES_H
