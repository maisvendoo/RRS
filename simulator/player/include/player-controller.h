//------------------------------------------------------------------------------
//
//      Player controller (пешая ходьба)
//      ТЗ "walking": выход из кабины, осмотр состава
//
//      Физика пешего режима: ускорение/трение к желаемому направлению,
//      спринт, присед (смена высоты глаз), прыжок, гравитация, проверка
//      земли лучом, подъём на ступеньки (до 0.4 м), скольжение вдоль
//      стен (движение проецируется на плоскость препятствия), запрет
//      подъёма по крутым склонам (> 45 град). Камера плавная.
//
//------------------------------------------------------------------------------

#ifndef     PLAYER_CONTROLLER_H
#define     PLAYER_CONTROLLER_H

#include    <collision-world.h>

#include    <QString>

#include    <cstddef>

//------------------------------------------------------------------------------
/// Пеший игрок
//------------------------------------------------------------------------------
class PlayerController
{
public:

    PlayerController() = default;

    /// Загрузка параметров из секции [Player] конфигурации
    void loadConfig(QString cfg_path);

    /// Шаг: wish_x/wish_y - желаемое движение в локальных осях (-1..1,
    /// x - вправо, y - вперёд), jump - прыжок, sprint - спринт,
    /// crouch - присед, yaw - направление взгляда, рад
    void step(double dt,
              double wish_x,
              double wish_y,
              bool jump,
              bool sprint,
              bool crouch,
              double yaw,
              collision::CollisionWorld* world);

    /// Позиция (центр капсулы), м
    const collision::Vec3f& getPosition() const;

    /// Высота глаз над центром (присед меняет), м
    double getEyeHeight() const;

    /// На земле
    bool isGrounded() const;

    /// Горизонтальная скорость, м/с
    double getSpeed() const;

    /// Вертикальная скорость, м/с (отрицательная - падение)
    double getVerticalVelocity() const;

    /// Приземлился на этом шаге (фронтом касания земли)
    bool justLanded() const;

    /// Скорость удара при приземлении, м/с (для проседания камеры)
    double getLandingImpact() const;

    /// В спринте (для расширения FOV камеры, ТЗ п.12)
    bool isSprinting() const;

    /// В приседе
    bool isCrouched() const;

    /// Телепорт (спавн/вход в кабину)
    void teleport(const collision::Vec3f& position);

    /// Смотрит в направлении yaw (рад). Yaw задаётся камерой
    void setYaw(double yaw);
    double getYaw() const;

private:

    collision::Vec3f position_ = {0.0f, 0.0f, 2.0f};
    collision::Vec3f velocity_ = {0.0f, 0.0f, 0.0f};

    /// Радиус капсулы, м
    float radius_ = 0.3f;

    /// Высота глаз над центром капсулы, м: стоя ~1.65 над землёй,
    /// в приседе ~0.95 (центр капсулы - radius_ над землёй)
    double eye_height_stand_ = 1.35;
    double eye_height_crouch_ = 0.65;

    double eye_height_ = 1.35;
    double eye_height_target_ = 1.35;

    bool grounded_ = false;
    bool crouched_ = false;
    bool sprinting_ = false;

    double yaw_ = 0.0;

    /// Приземление на последнем шаге (фронтом перехода земля/воздух)
    bool just_landed_ = false;
    double landing_impact_ = 0.0;

    /// Скорости, м/с
    double walk_speed_ = 1.6;
    double sprint_speed_ = 5.0;
    double crouch_speed_ = 0.8;

    /// Ускорение/трение, м/с^2
    double accelerate_ = 30.0;
    double friction_ = 12.0;

    /// Прыжок: начальная скорость, м/с
    double jump_speed_ = 4.5;

    /// Гравитация, м/с^2
    double gravity_ = 9.81;

    /// Высота ступеньки, м
    double step_height_ = 0.4;
};

#endif // PLAYER_CONTROLLER_H
