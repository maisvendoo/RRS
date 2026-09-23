//------------------------------------------------------------------------------
//
//      Player controller (пешая ходьба)
//
//------------------------------------------------------------------------------

#include    "player-controller.h"

#include    <CfgReader.h>

#include    <algorithm>
#include    <cmath>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PlayerController::loadConfig(QString cfg_path)
{
    CfgReader cfg;
    if (!cfg.load(cfg_path))
        return;

    const QString sec = "Player";

    double dvalue = 0.0;
    if (cfg.getDouble(sec, "Radius", dvalue))
        radius_ = static_cast<float>(dvalue);

    cfg.getDouble(sec, "WalkSpeed", walk_speed_);
    cfg.getDouble(sec, "SprintSpeed", sprint_speed_);
    cfg.getDouble(sec, "CrouchSpeed", crouch_speed_);
    cfg.getDouble(sec, "Acceleration", accelerate_);
    cfg.getDouble(sec, "Friction", friction_);
    cfg.getDouble(sec, "JumpSpeed", jump_speed_);
    cfg.getDouble(sec, "Gravity", gravity_);
    cfg.getDouble(sec, "StepHeight", step_height_);
    cfg.getDouble(sec, "EyeHeightStand", eye_height_stand_);
    cfg.getDouble(sec, "EyeHeightCrouch", eye_height_crouch_);

    eye_height_ = eye_height_stand_;
    eye_height_target_ = eye_height_stand_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PlayerController::step(double dt,
                            double wish_x,
                            double wish_y,
                            bool jump,
                            bool sprint,
                            bool crouch,
                            double yaw,
                            collision::CollisionWorld* world)
{
    yaw_ = yaw;
    sprinting_ = sprint && !crouch;

    //--- Присед: плавная смена высоты глаз (ТЗ, п.4, 16) ---
    // Вставание из приседа - только если над головой свободно:
    // луч вверх до макушки стоя (глаза + запас на голову)
    if (!crouch && crouched_ && world != nullptr)
    {
        collision::Vec3f point;
        collision::Vec3f normal;

        const collision::Vec3f up(0.0f, 0.0f, 1.0f);
        const float headroom = static_cast<float>(eye_height_stand_ + 0.15);

        if (world->raycast(position_, up, headroom, point, normal))
            crouch = true;  // Потолок: остаёмся в приседе
    }

    crouched_ = crouch;
    eye_height_target_ = crouch ? eye_height_crouch_ : eye_height_stand_;
    eye_height_ += (eye_height_target_ - eye_height_) *
            std::min(1.0, 10.0 * dt);

    //--- Желаемое направление в мире ---
    const double len = std::sqrt(wish_x * wish_x + wish_y * wish_y);

    double wish_dir_x = 0.0;
    double wish_dir_y = 0.0;

    if (len > 1e-3)
    {
        wish_x /= len;
        wish_y /= len;

        const double cos_y = std::cos(yaw);
        const double sin_y = std::sin(yaw);

        // Локальное x - вправо, y - вперёд; мировые оси XZ
        wish_dir_x = wish_x * cos_y - wish_y * sin_y;
        wish_dir_y = wish_x * sin_y + wish_y * cos_y;
    }

    //--- Целевая скорость (ТЗ, п.2-4) ---
    double target_speed = walk_speed_;

    if (crouch)
        target_speed = crouch_speed_;
    else if (sprint)
        target_speed = sprint_speed_;

    //--- Проверка земли лучом вниз ---
    const bool was_grounded = grounded_;
    grounded_ = false;
    just_landed_ = false;

    if (world != nullptr)
    {
        collision::Vec3f point;
        collision::Vec3f normal;

        const collision::Vec3f down(0.0f, 0.0f, -1.0f);
        const float probe = radius_ + 0.15f;

        // Дальний луч: страховка от проваливания (маршруты без
        // colliders.conf, дыры в геометрии) - ищем землю до 100 м ниже
        bool hit = world->raycast(position_, down, probe, point, normal);

        if (!hit && velocity_.z < 0.0f)
        {
            hit = world->raycast(position_, down, 100.0f, point, normal);
        }

        // Совсем нет геометрии - бесконечный пол на z = 0 (как без мира)
        if (!hit && position_.z <= radius_)
        {
            position_.z = radius_;
            grounded_ = true;
            velocity_.z = 0.0f;
        }

        if (hit)
        {
            // Крутые склоны не держат (ТЗ, п.9): нормаль близка
            // к вертикали - стоять можно
            if (normal.z > 0.7)
            {
                grounded_ = true;

                // Приземление (ТЗ, п.13): фронт воздух -> земля;
                // скорость удара - вертикальная скорость до касания
                if (!was_grounded && velocity_.z < 0.0f)
                {
                    just_landed_ = true;
                    landing_impact_ = -static_cast<double>(velocity_.z);
                }

                // Стоим на поверхности
                position_.z = point.z + radius_;
                velocity_.z = 0.0f;
            }
        }
    }
    else
    {
        // Без мира: бесконечный пол на z = 0
        if (position_.z <= radius_)
        {
            if (!was_grounded && velocity_.z < 0.0f)
            {
                just_landed_ = true;
                landing_impact_ = -static_cast<double>(velocity_.z);
            }

            position_.z = radius_;
            grounded_ = true;
            velocity_.z = 0.0f;
        }
    }

    //--- Прыжок (ТЗ, п.5) ---
    if (jump && grounded_)
    {
        velocity_.z = static_cast<float>(jump_speed_);
        grounded_ = false;
    }

    //--- Горизонтальное движение: ускорение и трение ---
    const double target_vx = wish_dir_x * target_speed;
    const double target_vy = wish_dir_y * target_speed;

    const double current_vx = velocity_.x;
    const double current_vy = velocity_.y;

    // Трение только на земле (в воздухе управление слабее)
    const double control = grounded_ ? 1.0 : 0.05;

    if (len > 1e-3)
    {
        velocity_.x += static_cast<float>(
                    (target_vx - current_vx) * accelerate_ * control * dt /
                    std::max(target_speed, 0.1));
        velocity_.y += static_cast<float>(
                    (target_vy - current_vy) * accelerate_ * control * dt /
                    std::max(target_speed, 0.1));
    }
    else if (grounded_)
    {
        // Трение к нулю
        const double decay = std::min(1.0, friction_ * dt);

        velocity_.x -= static_cast<float>(velocity_.x * decay);
        velocity_.y -= static_cast<float>(velocity_.y * decay);
    }

    //--- Столкновения со стенами: лучи вперёд, скольжение (ТЗ, п.15) ---
    if (world != nullptr)
    {
        collision::Vec3f point;
        collision::Vec3f normal;

        const float vx = velocity_.x;
        const float vy = velocity_.y;

        const float hspeed = std::sqrt(vx * vx + vy * vy);

        if (hspeed > 1e-3f)
        {
            const collision::Vec3f dir(vx / hspeed, vy / hspeed, 0.0f);

            if (world->raycast(position_, dir, radius_ + 0.1f,
                               point, normal))
            {
                // Стена: проекция скорости на плоскость стены
                const float dot = velocity_.x * normal.x +
                                  velocity_.y * normal.y;

                if (dot < 0.0f)
                {
                    velocity_.x -= dot * normal.x;
                    velocity_.y -= dot * normal.y;
                }

                // Попытка ступеньки: приподняться и пройти (ТЗ, п.8).
                // Поднимаем только если под приподнятой позицией есть
                // опора (луч вниз не длиннее step_height + радиус),
                // иначе подъём превращался бы в «лифт» вдоль стены
                const collision::Vec3f lifted(position_.x,
                                              position_.y,
                                              position_.z +
                                              static_cast<float>(step_height_));

                collision::Vec3f p2;
                collision::Vec3f n2;

                if (!world->raycast(lifted, dir, radius_ + 0.15f, p2, n2))
                {
                    collision::Vec3f ground_p;
                    collision::Vec3f ground_n;

                    const collision::Vec3f down(0.0f, 0.0f, -1.0f);
                    const float ground_probe =
                            static_cast<float>(step_height_) + radius_;

                    // Крутые склоны не держат (как и в проверке земли)
                    if (world->raycast(lifted, down, ground_probe,
                                       ground_p, ground_n) &&
                            ground_n.z > 0.7)
                    {
                        position_.z = lifted.z;
                    }
                }
            }
        }
    }

    //--- Гравитация ---
    if (!grounded_)
    {
        velocity_.z -= static_cast<float>(gravity_ * dt);
    }

    //--- Интегрирование ---
    position_.x += velocity_.x * static_cast<float>(dt);
    position_.y += velocity_.y * static_cast<float>(dt);
    position_.z += velocity_.z * static_cast<float>(dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
const collision::Vec3f& PlayerController::getPosition() const
{
    return position_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PlayerController::getEyeHeight() const
{
    return eye_height_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool PlayerController::isGrounded() const
{
    return grounded_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PlayerController::getSpeed() const
{
    return std::sqrt(static_cast<double>(velocity_.x) * velocity_.x +
                     static_cast<double>(velocity_.y) * velocity_.y);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PlayerController::getVerticalVelocity() const
{
    return velocity_.z;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool PlayerController::justLanded() const
{
    return just_landed_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PlayerController::getLandingImpact() const
{
    return landing_impact_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool PlayerController::isSprinting() const
{
    return sprinting_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool PlayerController::isCrouched() const
{
    return crouched_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PlayerController::setYaw(double yaw)
{
    yaw_ = yaw;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double PlayerController::getYaw() const
{
    return yaw_;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void PlayerController::teleport(const collision::Vec3f& pos)
{
    position_ = pos;
    velocity_ = collision::Vec3f(0.0f, 0.0f, 0.0f);
}
