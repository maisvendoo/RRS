//------------------------------------------------------------------------------
//
//      Camera motion from physics (физическая реакция машиниста)
//      ТЗ "Физическая реакция машиниста"
//
//      Камера не «трясётся» сама: она сидит на физическом теле. Фильтр
//      «шея машиниста» (пружина-демпфер) преобразует ускорения кузова
//      (вертикальные/поперечные), тангаж и крен в смещение и наклон
//      головы/камеры с инерцией, ограничением амплитуды и затуханием
//      на стоянке. Реакция продольная - от продольного ускорения ОДУ.
//
//------------------------------------------------------------------------------

#ifndef     VEHICLE_CAMERA_MOTION_H
#define     VEHICLE_CAMERA_MOTION_H

#include    <QString>

#include    <cstddef>

//------------------------------------------------------------------------------
/// Реакция тела машиниста на движение кузова
//------------------------------------------------------------------------------
class CameraMotionFromPhysics
{
public:

    CameraMotionFromPhysics() = default;

    /// Загрузка секции [CameraMotion]
    void loadConfig(QString cfg_path);

    /// Шаг: вертикальное ускорение кузова (м/с^2), поперечное,
    /// продольное (от изменения скорости), тангаж/крен кузова (рад)
    void step(double dt,
              double vertical_accel,
              double lateral_accel,
              double longitudinal_accel,
              double body_pitch,
              double body_roll);

    /// Смещение головы/камеры, м: X - продольное, Y - поперечное, Z - верт.
    double getOffsetX() const;
    double getOffsetY() const;
    double getOffsetZ() const;

    /// Наклон камеры, рад: реакция тела + физический угол кузова
    /// (тангаж/крен), суммарно ограничено max_tilt
    double getTiltRoll() const;
    double getTiltPitch() const;

    QString getDebugMsg() const;

private:

    /// Жёсткость «шеи» (1/с^2): ниже - более вялые колебания головы
    double neck_stiffness = 60.0;
    double neck_damping = 8.0;

    /// Ограничение смещения головы, м
    double max_offset = 0.08;

    /// Ограничение наклона, рад
    double max_tilt = 0.08;

    // Состояния пружины-демпфера головы (по трём осям)
    double x = 0.0;  double dx = 0.0;
    double y = 0.0;  double dy = 0.0;
    double z = 0.0;  double dz = 0.0;

    // Физические углы кузова из последнего шага, рад: суммируются
    // с наклоном головы в общем лимите max_tilt
    double body_pitch_ = 0.0;
    double body_roll_ = 0.0;
};

#endif // VEHICLE_CAMERA_MOTION_H
