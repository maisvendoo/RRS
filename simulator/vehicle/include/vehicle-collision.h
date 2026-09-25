//------------------------------------------------------------------------------
//
//      Vehicle collision bodies (colliders over Jolt Physics)
//
//------------------------------------------------------------------------------

#ifndef     VEHICLE_COLLISION_H
#define     VEHICLE_COLLISION_H

#include    <collision-object.h>

#include    <simulation-lod.h>

#include    <QString>
#include    <cstddef>
#include    <cstdint>
#include    <vector>

namespace collision
{
    class CollisionWorld;
}

struct profile_point_t;

//------------------------------------------------------------------------------
/// Коллайдеры подвижной единицы: кузов, тележки, колёсные пары.
/// Тела кинематические и следуют за положением ПЕ на траектории
/// (динамика схода - дни 12-13 плана)
//------------------------------------------------------------------------------
class VehicleCollision
{
public:

    VehicleCollision() = default;

    /// Прочитать секцию Collision конфига ПЕ; умолчания - из параметров ПЕ
    void loadConfig(QString cfg_path, double length, std::size_t num_axis,
                    double wheel_diameter);

    /// Коллайдеры включены конфигом
    bool isEnabled() const;

    /// Создать тела в мире коллизий
    void createBodies(collision::CollisionWorld& world,
                      collision::GroupId group,
                      void* user_data);

    /// Синхронизировать положение тел с положением ПЕ на траектории
    void syncPose(const profile_point_t& profile_point);

    /// Удалить тела из мира
    void removeBodies(collision::CollisionWorld& world);

    /// Тела созданы
    bool hasBodies() const;

    /// Уровень детализации ПС (ТЗ "Коллизии" п.27): при L2 и выше
    /// коллайдеры тележек/колёсных пар убираются из мира (остаётся
    /// один box кузова), при возврате на L0/L1 пересоздаются
    /// на последней позе ПС. Уровень по умолчанию - L0_Full
    void setLod(perf::SimLOD lod, collision::CollisionWorld& world);
    perf::SimLOD getLod() const;

private:

    /// Тип части набора коллайдеров (для LOD-упрощения)
    enum class PartKind : std::uint8_t
    {
        Body = 0,   ///< Кузов (не убирается никогда)
        Bogie,      ///< Тележка
        Wheel       ///< Колёсная пара
    };

    /// Часть набора коллайдеров: тело + его смещение в локальных осях ПЕ.
    /// Хранит данные создания: они нужны для пересоздания тел
    /// при возврате с L2 на L0/L1
    struct Part
    {
        collision::CollisionObject object;
        collision::Vec3f local_offset;  ///< (вдоль пути, вправо, вверх), м

        collision::CollisionShape shape;
        collision::Layer layer = collision::Layer::Train;
        collision::CollisionProfile profile;
        collision::GroupId group = collision::no_group;
        void* user_data = nullptr;
        PartKind kind = PartKind::Body;

        collision::Vec3f last_position;    ///< последняя поза (для
        collision::Quatf last_rotation;   ///< восстановления после LOD)
    };

    void addPart(collision::CollisionWorld& world,
                 const collision::CollisionShape& shape,
                 collision::Layer layer,
                 const collision::CollisionProfile& profile,
                 const collision::Vec3f& local_offset,
                 collision::GroupId group,
                 void* user_data,
                 PartKind kind);

    /// Пересоздать тело части на последней известной позе
    void recreatePart(collision::CollisionWorld& world, Part& part);

    bool enabled = true;

    // Кузов
    collision::Vec3f body_half_extents = {6.96f, 1.55f, 1.6f};
    float body_offset_z = 2.0f;         ///< Подъём центра кузова над УГР

    // Тележки
    int num_bogies = 2;
    collision::Vec3f bogie_half_extents = {1.8f, 1.15f, 0.5f};
    float bogie_offset = 4.86f;         ///< От центра ПЕ до крайней тележки
    float bogie_offset_z = 0.85f;

    // Колёсные пары
    float wheelset_spacing = 2.4f;      ///< Межосевое расстояние в тележке
    float wheelset_half_width = 1.0f;   ///< Половина ширины колёсной пары
    float wheel_radius = 0.475f;

    std::size_t num_axis = 4;

    /// Текущий уровень детализации (влияет на набор тел)
    perf::SimLOD lod = perf::SimLOD::L0_Full;

    std::vector<Part> parts;
};

#endif // VEHICLE_COLLISION_H
