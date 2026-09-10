//------------------------------------------------------------------------------
//
//      Camera motion from physics (физическая реакция машиниста)
//
//------------------------------------------------------------------------------

#include    "vehicle-camera-motion.h"

#include    <CfgReader.h>

#include    <algorithm>
#include    <cmath>

namespace
{

/// Пружина-демпфер головы: голова отстаёт от кузова на ускорении
void integrateHead(double& pos, double& vel, double accel,
                   double stiffness, double damping, double limit,
                   double dt)
{
    // Голова стремится к нулю (нейтральное положение),
    // ускорение кузова - возмущающая сила
    const double a = -stiffness * pos - damping * vel - 2.5 * accel;

    vel += a * dt;
    pos += vel * dt;

    pos = std::min(std::max(pos, -limit), limit);
}

} // namespace

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraMotionFromPhysics::loadConfig(QString cfg_path)
{
    CfgReader cfg;
    if (!cfg.load(cfg_path))
        return;

    const QString sec = "CameraMotion";

    cfg.getDouble(sec, "NeckStiffness", neck_stiffness);
    cfg.getDouble(sec, "NeckDamping", neck_damping);
    cfg.getDouble(sec, "MaxOffset", max_offset);
    cfg.getDouble(sec, "MaxTilt", max_tilt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void CameraMotionFromPhysics::step(double dt,
                                   double vertical_accel,
                                   double lateral_accel,
                                   double longitudinal_accel,
                                   double body_pitch,
                                   double body_roll)
{
    // Физические углы кузова входят в наклон головы/камеры
    body_pitch_ = body_pitch;
    body_roll_ = body_roll;

    if (dt <= 0.0)
        return;

    // Пружина «шеи» (k=60) численно неустойчива при dt > 0.05 с:
    // большие шаги режем на подшаги не длиннее 0.05 с
    const auto substeps = std::max<std::size_t>(
                static_cast<std::size_t>(std::ceil(dt / 0.05)), 1);
    const double h = dt / static_cast<double>(substeps);

    for (std::size_t i = 0; i < substeps; ++i)
    {
        integrateHead(z, dz, vertical_accel, neck_stiffness, neck_damping,
                      max_offset, h);
        integrateHead(y, dy, lateral_accel, neck_stiffness, neck_damping,
                      max_offset, h);
        integrateHead(x, dx, longitudinal_accel, neck_stiffness, neck_damping,
                      max_offset, h);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double CameraMotionFromPhysics::getOffsetX() const
{
    return x;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double CameraMotionFromPhysics::getOffsetY() const
{
    return y;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double CameraMotionFromPhysics::getOffsetZ() const
{
    return z;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double CameraMotionFromPhysics::getTiltRoll() const
{
    // Наклон головы пропорционален поперечному отклонению,
    // плюс физический крен кузова; общий лимит max_tilt
    const double head_tilt = -y / std::max(max_offset, 1e-3) * max_tilt;
    return std::clamp(head_tilt + body_roll_, -max_tilt, max_tilt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double CameraMotionFromPhysics::getTiltPitch() const
{
    // Наклон головы от продольного отклонения плюс физический
    // тангаж кузова; общий лимит max_tilt
    const double head_tilt = -x / std::max(max_offset, 1e-3) * max_tilt;
    return std::clamp(head_tilt + body_pitch_, -max_tilt, max_tilt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString CameraMotionFromPhysics::getDebugMsg() const
{
    return QString("Camera body: dx=%1 dy=%2 dz=%3 m, roll=%4 pitch=%5 rad")
            .arg(x, 0, 'f', 3)
            .arg(y, 0, 'f', 3)
            .arg(z, 0, 'f', 3)
            .arg(getTiltRoll(), 0, 'f', 4)
            .arg(getTiltPitch(), 0, 'f', 4);
}
