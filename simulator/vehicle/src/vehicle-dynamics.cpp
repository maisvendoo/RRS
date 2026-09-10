//------------------------------------------------------------------------------
//
//      Vehicle vertical dynamics (multibody: wheelset - bogie - body)
//
//------------------------------------------------------------------------------

#include    "vehicle-dynamics.h"

#include    "physics.h"

#include    <CfgReader.h>

#include    <algorithm>
#include    <cmath>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleVerticalDynamics::loadConfig(QString cfg_path,
                                         double full_mass_,
                                         std::size_t num_axis_,
                                         double length_)
{
    enabled = true;
    full_mass = full_mass_;
    num_axis = std::max<std::size_t>(num_axis_, 2);
    length = std::max(length_, 4.0);

    // Геометрия по умолчанию: крайняя тележка - у конца ПЕ,
    // оси внутри тележки - с шагом WheelsetSpacing
    num_bogies = std::max<std::size_t>(num_axis / 2, 1);

    double bogie_offset_default = std::max(length * 0.5 - 2.1, 1.0);
    double wheelset_spacing = 1.85;

    primary.stiffness = 1200000.0;
    primary.damping = 25000.0;
    primary.stroke = 0.06;

    secondary.stiffness = 450000.0;
    secondary.damping = 18000.0;
    secondary.stroke = 0.09;

    wheelset_mass = 1400.0;
    bogie_mass = 2200.0;
    contact_stiffness = 1.0e9;
    contact_damping = 2.0e5;
    pitch_radius = 0.29 * length;
    roll_radius = 1.15;
    substep = 0.001;

    CfgReader cfg;
    if (!cfg.load(cfg_path))
        return;

    const QString sec = "Suspension";

    cfg.getBool(sec, "Enabled", enabled);

    double value = 0.0;
    int ivalue = 0;

    if (cfg.getDouble(sec, "PrimaryStiffness", value))
        primary.stiffness = value;
    if (cfg.getDouble(sec, "PrimaryDamping", value))
        primary.damping = value;
    if (cfg.getDouble(sec, "PrimaryStroke", value))
        primary.stroke = value;

    if (cfg.getDouble(sec, "SecondaryStiffness", value))
        secondary.stiffness = value;
    if (cfg.getDouble(sec, "SecondaryDamping", value))
        secondary.damping = value;
    if (cfg.getDouble(sec, "SecondaryStroke", value))
        secondary.stroke = value;

    if (cfg.getDouble(sec, "WheelsetMass", value))
        wheelset_mass = value;
    if (cfg.getDouble(sec, "BogieMass", value))
        bogie_mass = value;
    if (cfg.getDouble(sec, "ContactStiffness", value))
        contact_stiffness = value;
    if (cfg.getDouble(sec, "ContactDamping", value))
        contact_damping = value;

    if (cfg.getDouble(sec, "BogieOffset", value))
        bogie_offset_default = value;
    if (cfg.getDouble(sec, "WheelsetSpacing", value))
        wheelset_spacing = value;
    if (cfg.getDouble(sec, "SecondaryHalfSpan", value))
        secondary_half_span = value;
    if (cfg.getDouble(sec, "PitchRadius", value))
        pitch_radius = value;
    if (cfg.getDouble(sec, "RollRadius", value))
        roll_radius = value;
    if (cfg.getDouble(sec, "PitchTorqueArm", value))
        pitch_torque_arm = std::max(value, 0.1);
    if (cfg.getDouble(sec, "Substep", value))
        substep = std::min(std::max(value, 0.0002), 0.005);

    // Детектор ударов осей о стыки (счётчик для звука JointImpact)
    if (cfg.getDouble(sec, "JointImpactThreshold", value))
        joint_impact_threshold = std::max(value, 1.0);
    if (cfg.getDouble(sec, "JointImpactRefractory", value))
        joint_impact_refractory = std::min(std::max(value, 0.01), 1.0);

    if (cfg.getInt(sec, "NumBogies", ivalue))
        num_bogies = static_cast<std::size_t>(std::max(ivalue, 1));

    primary.stop_stiffness = 8.0 * primary.stiffness;
    secondary.stop_stiffness = 8.0 * secondary.stiffness;

    // Раскладка осей по тележкам (как в коллайдерах ПЕ)
    axle_offset.assign(num_axis, 0.0);
    axle_bogie.assign(num_axis, 0);

    bogie_offset.assign(num_bogies, 0.0);
    if (num_bogies > 1)
    {
        for (std::size_t k = 0; k < num_bogies; ++k)
        {
            bogie_offset[k] = -bogie_offset_default +
                    2.0 * bogie_offset_default * static_cast<double>(k) /
                    static_cast<double>(num_bogies - 1);
        }
    }

    std::size_t axle_idx = 0;
    for (std::size_t k = 0; k < num_bogies && axle_idx < num_axis; ++k)
    {
        const std::size_t axles_here = num_axis / num_bogies +
                ((k < num_axis % num_bogies) ? 1 : 0);

        for (std::size_t j = 0; j < axles_here && axle_idx < num_axis; ++j, ++axle_idx)
        {
            axle_offset[axle_idx] = bogie_offset[k] + wheelset_spacing *
                    (static_cast<double>(j) -
                     0.5 * static_cast<double>(axles_here - 1));
            axle_bogie[axle_idx] = k;
        }
    }

    // Состояния
    wheelset_z.assign(num_axis, Dof());
    wheelset_roll.assign(num_axis, Dof());
    bogie_z.assign(num_bogies, Dof());
    bogie_roll.assign(num_bogies, Dof());
    body_z = Dof();
    body_pitch = Dof();
    body_roll = Dof();

    // Рабочие буферы
    rail_z.assign(num_axis * 2, 0.0);
    rail_rate.assign(num_axis * 2, 0.0);
    contact_force.assign(num_axis * 2, 0.0);
    primary_on_axle.assign(num_axis, 0.0);
    primary_on_bogie.assign(num_bogies, 0.0);
    primary_torque_on_axle.assign(num_axis, 0.0);
    primary_torque_on_bogie.assign(num_bogies, 0.0);
    secondary_on_bogie.assign(num_bogies, 0.0);
    secondary_torque_on_bogie.assign(num_bogies, 0.0);

    axle_load_factor.assign(num_axis, 1.0);
    prev_rail_z.assign(num_axis * 2, 0.0);
    static_axle_share.assign(num_axis, 1.0);

    // Счётчики ударов осей о стыки (монотонные, сброса нет)
    axle_joint_count.assign(num_axis, 0UL);
    axle_joint_refract.assign(num_axis, 0.0);

    resetComfortMetrics();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehicleVerticalDynamics::isEnabled() const
{
    return enabled;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehicleVerticalDynamics::isReady() const
{
    return ready && static_cast<bool>(rail_height);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleVerticalDynamics::setRailHeightSource(RailHeightFn fn)
{
    rail_height = std::move(fn);
    ready = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleVerticalDynamics::setStaticAxleLoads(const std::vector<double>& axle_shares)
{
    if (axle_shares.size() != num_axis)
        return;

    static_axle_share = axle_shares;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleVerticalDynamics::applyFlatImpact(std::size_t axle, double drop_speed)
{
    if (axle >= wheelset_z.size())
        return;

    if (!enabled)
        return;

    // Колесо "падает" в лыску: отрицательная вертикальная скорость,
    // контактная пружина Герца затем отбивает колёсную пару вверх.
    // Импульс естественно уходит в подвеску и кузов
    wheelset_z[axle].vel -= drop_speed;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleVerticalDynamics::step(double dt,
                                   double velocity,
                                   std::int8_t dir,
                                   double train_coord,
                                   double full_mass_,
                                   double cant_mm,
                                   double longitudinal_accel)
{
    if (!enabled || !isReady() || dt <= 0.0)
        return;

    full_mass = std::max(full_mass_, 1000.0);

    // Подрессоренная масса: пересчёт под текущую загрузку ПЕ
    body_mass = std::max(full_mass -
                         static_cast<double>(num_axis) * wheelset_mass -
                         static_cast<double>(num_bogies) * bogie_mass,
                         0.2 * full_mass);

    // Жёсткий контакт требует малого шага: дробим шаг модели
    const auto substeps = static_cast<std::size_t>(
                std::min(std::ceil(dt / substep), 1000.0));

    const double h = dt / static_cast<double>(substeps);

    for (std::size_t i = 0; i < substeps; ++i)
    {
        integrateSubstep(h, velocity, dir, train_coord,
                         cant_mm, longitudinal_accel);
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleVerticalDynamics::integrateSubstep(double h,
                                               double velocity,
                                               std::int8_t dir,
                                               double train_coord,
                                               double cant_mm,
                                               double longitudinal_accel)
{
    const double g = Physics::g;

    // Координата пути центра ПЕ: путь = dir * координата поезда (как
    // в Train::slotStep при обновлении контроллера ПЕ)
    const double center_path = static_cast<double>(dir) * train_coord;

    const double half_gauge = contact_half_span;
    const double half_primary = primary_half_span;
    const double half_secondary = secondary_half_span;

    const double base_axle_load = full_mass * g / static_cast<double>(num_axis);

    const double kH = 0.5 * contact_stiffness;      ///< на колесо
    const double cH = 0.5 * contact_damping;

    // Продвижение вдоль пути за подшаг (скорость - по координате пути)
    const double travel = velocity * h;

    // Возвышение наружного рельса (Б16): разность высот рельсов
    // h_cant = cant_mm/1000 задаёт статический крен колёсной пары
    // theta ~ h_cant/(2*b). Среднюю высоту рельсов не смещаем (модель
    // в отклонениях): сторона 0 (левая) +h/2, сторона 1 (правая) -h/2.
    // Знак cant: "+" - левый рельс выше. На отводе возвышения смена
    // z по пути сама даёт скорость крена (возбуждение подвески)
    const double cant_half_m = 0.5 * cant_mm * 1.0e-3;

    //--- 1. Вход: высота рельса под каждым колесом ---

    for (std::size_t i = 0; i < num_axis; ++i)
    {
        const double axle_path = center_path +
                static_cast<double>(dir) * axle_offset[i] + travel;

        for (int s = 0; s < 2; ++s)
        {
            const double z = rail_height(axle_path, s) +
                    ((s == 0) ? cant_half_m : -cant_half_m);
            const std::size_t idx = i * 2 + static_cast<std::size_t>(s);

            rail_z[idx] = z;

            // Скорость профиля под колесом: конечная разность по подшагу
            rail_rate[idx] = (z - prev_rail_z[idx]) / h;
            prev_rail_z[idx] = z;
        }
    }

    //--- 2. Силы контакта колеса с рельсом (Герц, односторонняя) ---
    // F = P0 + kH * delta + cH * delta', delta = z_rail - z_wheel_corner.
    // Отклонение силы (вычитаем статическую долю) входит в уравнения

    for (std::size_t i = 0; i < num_axis; ++i)
    {
        // Статическая нагрузка оси с учётом смещения центра масс
        const double static_axle_load = base_axle_load * static_axle_share[i];
        const double static_wheel_load = 0.5 * static_axle_load;

        const double wheel_z = wheelset_z[i].pos;
        const double wheel_roll = wheelset_roll[i].pos;
        const double wheel_vz = wheelset_z[i].vel;
        const double wheel_vroll = wheelset_roll[i].vel;

        for (int s = 0; s < 2; ++s)
        {
            const double arm = (s == 0) ? half_gauge : -half_gauge;

            const double corner_pos = wheel_z + wheel_roll * arm;
            const double corner_vel = wheel_vz + wheel_vroll * arm;

            const double delta = rail_z[i * 2 + s] - corner_pos;
            const double delta_rate = rail_rate[i * 2 + s] - corner_vel;

            double force = static_wheel_load + kH * delta + cH * delta_rate;

            // Односторонний контакт: рельс не тянет колесо вниз
            if (force < 0.0)
                force = 0.0;

            contact_force[i * 2 + s] = force;
        }
    }

    //--- 3. Первая ступень: колёсная пара <-> тележка ---

    std::fill(primary_on_axle.begin(), primary_on_axle.end(), 0.0);
    std::fill(primary_on_bogie.begin(), primary_on_bogie.end(), 0.0);
    std::fill(primary_torque_on_axle.begin(), primary_torque_on_axle.end(), 0.0);
    std::fill(primary_torque_on_bogie.begin(), primary_torque_on_bogie.end(), 0.0);

    for (std::size_t i = 0; i < num_axis; ++i)
    {
        const std::size_t k = axle_bogie[i];

        for (int s = 0; s < 2; ++s)
        {
            const double arm = (s == 0) ? half_primary : -half_primary;

            const double wheel_corner = wheelset_z[i].pos +
                    wheelset_roll[i].pos * arm;
            const double bogie_corner = bogie_z[k].pos +
                    bogie_roll[k].pos * arm;

            const double wheel_corner_rate = wheelset_z[i].vel +
                    wheelset_roll[i].vel * arm;
            const double bogie_corner_rate = bogie_z[k].vel +
                    bogie_roll[k].vel * arm;

            // Сжатие пружины: колесо выше тележки - пружина сжата
            const double deflection = wheel_corner - bogie_corner;
            const double deflection_rate = wheel_corner_rate - bogie_corner_rate;

            // Сила на тележку (вверх), на колесо - вниз
            const double force = 0.5 * primary.stiffness * deflection +
                                 0.5 * primary.damping * deflection_rate +
                                 bumpStop(deflection, primary);

            primary_on_bogie[k] += force;
            primary_on_axle[i] -= force;

            primary_torque_on_bogie[k] += force * arm;
            primary_torque_on_axle[i] -= force * arm;
        }
    }

    //--- 4. Вторая ступень: тележка <-> кузов ---

    std::fill(secondary_on_bogie.begin(), secondary_on_bogie.end(), 0.0);
    std::fill(secondary_torque_on_bogie.begin(),
              secondary_torque_on_bogie.end(), 0.0);

    double secondary_on_body = 0.0;
    double pitch_torque_on_body = 0.0;
    double roll_torque_on_body = 0.0;

    for (std::size_t k = 0; k < num_bogies; ++k)
    {
        const double x_b = bogie_offset[k];

        for (int s = 0; s < 2; ++s)
        {
            const double arm = (s == 0) ? half_secondary : -half_secondary;

            // Узел кузова над тележкой (малые углы)
            const double body_node = body_z.pos + body_pitch.pos * x_b +
                    body_roll.pos * arm;
            const double body_node_rate = body_z.vel + body_pitch.vel * x_b +
                    body_roll.vel * arm;

            const double bogie_corner = bogie_z[k].pos +
                    bogie_roll[k].pos * arm;
            const double bogie_corner_rate = bogie_z[k].vel +
                    bogie_roll[k].vel * arm;

            // Сжатие: сторона тележки выше узла кузова
            const double deflection = bogie_corner - body_node;
            const double deflection_rate = bogie_corner_rate - body_node_rate;

            // Сила на кузов (вверх), на тележку - вниз
            const double force = 0.5 * secondary.stiffness * deflection +
                                 0.5 * secondary.damping * deflection_rate +
                                 bumpStop(deflection, secondary);

            secondary_on_body += force;
            secondary_on_bogie[k] -= force;

            pitch_torque_on_body += force * x_b;
            roll_torque_on_body += force * arm;
            secondary_torque_on_bogie[k] -= force * arm;
        }
    }

    //--- 5. Уравнения движения (в отклонениях: гравитация сокращена) ---

    // Колёсные пары: подпрыгивание и крен
    for (std::size_t i = 0; i < num_axis; ++i)
    {
        const double contact_dev = contact_force[i * 2] +
                                   contact_force[i * 2 + 1] -
                                   base_axle_load * static_axle_share[i];

        wheelset_z[i].acc = (contact_dev + primary_on_axle[i]) / wheelset_mass;

        const double contact_torque_dev =
                (contact_force[i * 2] - contact_force[i * 2 + 1]) * half_gauge;

        const double roll_inertia = wheelset_mass * 0.5;
        wheelset_roll[i].acc = (contact_torque_dev +
                               primary_torque_on_axle[i]) / roll_inertia;
    }

    // Тележки
    for (std::size_t k = 0; k < num_bogies; ++k)
    {
        bogie_z[k].acc = (primary_on_bogie[k] + secondary_on_bogie[k]) /
                         bogie_mass;

        const double roll_inertia = bogie_mass * 0.8;
        bogie_roll[k].acc = (primary_torque_on_bogie[k] +
                             secondary_torque_on_bogie[k]) / roll_inertia;
    }

    // Кузов
    body_z.acc = secondary_on_body / body_mass;

    // Инерционный тангаж (ProdVertKoleb п.4): продольное ускорение ПЕ
    // действует на ЦМ кузова, расположенный выше центра букс:
    //     M_pitch = m_body * a * h_cm
    // ускорение (+ вперёд) - поднимает нос, торможение (a < 0) -
    // кузов клюёт носом. Подвеска сопротивляется через момент II ступени
    const double inertia_pitch_torque = body_mass * longitudinal_accel *
            pitch_torque_arm;

    body_pitch.acc = (pitch_torque_on_body + inertia_pitch_torque) /
            (body_mass * pitch_radius * pitch_radius);
    body_roll.acc = roll_torque_on_body / (body_mass * roll_radius * roll_radius);

    //--- 6. Интегрирование (полунеявный Эйлер - симплектичный) ---

    auto integrate = [h](Dof& dof)
    {
        dof.vel += dof.acc * h;
        dof.pos += dof.vel * h;
    };

    for (std::size_t i = 0; i < num_axis; ++i)
    {
        integrate(wheelset_z[i]);
        integrate(wheelset_roll[i]);
    }

    for (std::size_t k = 0; k < num_bogies; ++k)
    {
        integrate(bogie_z[k]);
        integrate(bogie_roll[k]);
    }

    integrate(body_z);
    integrate(body_pitch);
    integrate(body_roll);

    //--- 7. Датчики ---

    // Ускорение кузова: фильтр нижних частот ~20 Гц
    const double alpha = std::min(1.0, h / 0.008);
    body_accel_filtered += alpha * (body_z.acc - body_accel_filtered);

    // Рывок: производная ускорения с ограничением выброса
    const double jerk_raw = (body_z.acc - prev_body_accel) / h;
    prev_body_accel = body_z.acc;

    const double jerk_alpha = std::min(1.0, h / 0.02);
    body_jerk += jerk_alpha * (jerk_raw - body_jerk);

    // Нагрузка на оси
    for (std::size_t i = 0; i < num_axis; ++i)
    {
        axle_load_factor[i] = (contact_force[i * 2] + contact_force[i * 2 + 1]) /
                              (base_axle_load * static_axle_share[i]);
    }

    //--- 7.1 Детектор ударов осей о стыки/дефекты пути ---
    // Заброс вертикального ускорения колёсной пары над порогом с
    // рефрактерностью = один удар; счётчик монотонный (дельта между
    // опросами звука - новые удары)

    for (std::size_t i = 0; i < num_axis; ++i)
    {
        axle_joint_refract[i] = std::max(0.0, axle_joint_refract[i] - h);

        const double abs_acc = std::abs(wheelset_z[i].acc);

        if (abs_acc > joint_impact_threshold &&
                axle_joint_refract[i] <= 0.0)
        {
            ++axle_joint_count[i];
            axle_joint_refract[i] = joint_impact_refractory;

            // Нормировка: заброс над порогом до 2x порога = 1.0
            last_joint_intensity = std::min(
                        (abs_acc - joint_impact_threshold) /
                        joint_impact_threshold, 1.0);
        }
    }

    //--- 8. Метрики качества езды (прореживание до ~10 мс) ---

    metrics_sample_timer += h;
    shock_refractory = std::max(0.0, shock_refractory - h);

    if (metrics_sample_timer >= 0.01)
    {
        metrics_sample_timer = 0.0;

        const double abs_accel = std::abs(body_accel_filtered);

        accel_window.push_back(abs_accel);
        accel_sq_sum += abs_accel * abs_accel;

        // Окно 5 с при прореживании 10 мс
        while (accel_window.size() > 500)
        {
            accel_sq_sum -= accel_window.front() * accel_window.front();
            accel_window.pop_front();
        }

        max_accel = std::max(max_accel, abs_accel);
        max_jerk = std::max(max_jerk, std::abs(body_jerk));

        // Сильный удар: ускорение выше 3 м/с^2, рефрактерность 0.5 с
        if (abs_accel > 3.0 && shock_refractory <= 0.0)
        {
            ++shock_count;
            shock_refractory = 0.5;
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleVerticalDynamics::bumpStop(double deflection,
                                         const SuspensionStage& stage)
{
    if (deflection > stage.stroke)
        return stage.stop_stiffness * (deflection - stage.stroke);

    if (deflection < -stage.stroke)
        return stage.stop_stiffness * (deflection + stage.stroke);

    return 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleVerticalDynamics::getBodyAcceleration() const
{
    return body_accel_filtered;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleVerticalDynamics::getBodyJerk() const
{
    return body_jerk;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleVerticalDynamics::getBodyVerticalPosition() const
{
    return body_z.pos;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleVerticalDynamics::getBodyPitch() const
{
    return body_pitch.pos;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleVerticalDynamics::getBodyRoll() const
{
    return body_roll.pos;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleVerticalDynamics::getBogieAcceleration(std::size_t bogie) const
{
    if (bogie >= bogie_z.size())
        return 0.0;

    return bogie_z[bogie].acc;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleVerticalDynamics::getAxleLoadFactor(std::size_t axle) const
{
    if (axle >= axle_load_factor.size())
        return 1.0;

    return axle_load_factor[axle];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehicleVerticalDynamics::isAxleUnloaded(std::size_t axle) const
{
    if (axle >= axle_load_factor.size())
        return false;

    return axle_load_factor[axle] < 0.05;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleVerticalDynamics::getRMSAcceleration() const
{
    if (accel_window.empty())
        return 0.0;

    return std::sqrt(std::max(accel_sq_sum, 0.0) /
                     static_cast<double>(accel_window.size()));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleVerticalDynamics::getMaxAcceleration() const
{
    return max_accel;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleVerticalDynamics::getMaxJerk() const
{
    return max_jerk;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
int VehicleVerticalDynamics::getShockCount() const
{
    return shock_count;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
unsigned long VehicleVerticalDynamics::getJointImpactCount() const
{
    unsigned long total = 0;

    for (unsigned long count : axle_joint_count)
        total += count;

    return total;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleVerticalDynamics::getLastJointImpactIntensity() const
{
    return last_joint_intensity;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleVerticalDynamics::getRideIndex() const
{
    // Приближение индекса Шперлинга без частотного взвешивания
    const double a_rms = getRMSAcceleration();

    if (a_rms < 1e-6)
        return 0.0;

    return 0.896 * std::pow(a_rms, 0.798);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleVerticalDynamics::resetComfortMetrics()
{
    accel_sq_sum = 0.0;
    accel_window.clear();
    max_accel = 0.0;
    max_jerk = 0.0;
    shock_count = 0;
    shock_refractory = 0.0;
    metrics_sample_timer = 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString VehicleVerticalDynamics::getDebugMsg() const
{
    QString msg = QString("Vertical: z=%1 m a=%2 m/s^2 jerk=%3 m/s^3 "
                          "pitch=%4 rad roll=%5 rad Wz=%6")
            .arg(body_z.pos, 0, 'f', 4)
            .arg(body_accel_filtered, 0, 'f', 2)
            .arg(body_jerk, 0, 'f', 1)
            .arg(body_pitch.pos, 0, 'f', 4)
            .arg(body_roll.pos, 0, 'f', 4)
            .arg(getRideIndex(), 0, 'f', 2);

    for (std::size_t i = 0; i < axle_load_factor.size(); ++i)
    {
        msg += QString(" Q%1=%2").arg(i).arg(axle_load_factor[i], 0, 'f', 2);
    }

    return msg;
}
