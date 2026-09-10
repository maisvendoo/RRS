//------------------------------------------------------------------------------
//
//      Vehicle lateral dynamics (hunting, conicity, creep, flange contact)
//
//------------------------------------------------------------------------------

#include    "vehicle-lateral-dynamics.h"

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
void VehicleLateralDynamics::loadConfig(QString cfg_path,
                                        double full_mass_,
                                        std::size_t num_axis_,
                                        double length_,
                                        double wheel_diameter)
{
    enabled = false;
    full_mass = full_mass_;
    num_axis = std::max<std::size_t>(num_axis_, 2);
    length = std::max(length_, 4.0);
    wheel_radius = std::max(0.5 * wheel_diameter, 0.2);

    num_bogies = std::max<std::size_t>(num_axis / 2, 1);

    double bogie_offset_default = std::max(length * 0.5 - 2.1, 1.0);
    double wheelset_spacing = 1.85;

    CfgReader cfg;
    if (!cfg.load(cfg_path))
        return;

    const QString sec = "LateralDynamics";

    cfg.getBool(sec, "Enabled", enabled);

    double value = 0.0;
    int ivalue = 0;

    if (cfg.getDouble(sec, "Conicity", value))
        conicity = std::min(std::max(value, 0.0), 0.5);
    if (cfg.getDouble(sec, "FlangeClearance", value))
        flange_clearance = value;
    if (cfg.getDouble(sec, "FlangeStiffness", value))
        flange_stiffness = value;
    if (cfg.getDouble(sec, "FlangeDamping", value))
        flange_damping = value;
    if (cfg.getDouble(sec, "FlangeAngle", value))
        flange_angle = std::min(std::max(value, 45.0), 80.0);

    if (cfg.getDouble(sec, "CreepLongitudinal", value))
        creep_longitudinal = value;
    if (cfg.getDouble(sec, "CreepLateral", value))
        creep_lateral = value;

    // Спиновый крип (ТЗ "Поперечная динамика", п.20): коэффициент по
    // литературе c_spin ~ f33·a·γ (f33 - продольный крип, a - полуось
    // контактного пятна ~7.6 мм, γ - коничность = параметр спина конуса)
    bool spin_set = false;

    if (cfg.getDouble(sec, "SpinCreepCoeff", value))
    {
        spin_creep_coeff = std::max(value, 0.0);
        spin_set = true;
    }

    if (cfg.getDouble(sec, "PrimaryLateralStiffness", value))
        primary_lateral_stiffness = value;
    if (cfg.getDouble(sec, "PrimaryLateralDamping", value))
        primary_lateral_damping = value;
    if (cfg.getDouble(sec, "PrimaryYawStiffness", value))
        primary_yaw_stiffness = value;
    if (cfg.getDouble(sec, "PrimaryYawFreePlay", value))
        primary_yaw_free_play = value;
    if (cfg.getDouble(sec, "BogieYawStiffness", value))
        bogie_yaw_stiffness = value;
    if (cfg.getDouble(sec, "BogieYawFreePlay", value))
        bogie_yaw_free_play = value;
    if (cfg.getDouble(sec, "SecondaryLateralStiffness", value))
        secondary_lateral_stiffness = value;
    if (cfg.getDouble(sec, "SecondaryLateralDamping", value))
        secondary_lateral_damping = value;
    if (cfg.getDouble(sec, "SideBearerClearance", value))
        side_bearer_clearance = value;
    if (cfg.getDouble(sec, "SideBearerStiffness", value))
        side_bearer_stiffness = value;

    if (cfg.getDouble(sec, "WheelsetMass", value))
        wheelset_mass = value;
    if (cfg.getDouble(sec, "BogieMass", value))
        bogie_mass = value;
    if (cfg.getDouble(sec, "WheelsetYawInertia", value))
        wheelset_yaw_inertia = value;
    if (cfg.getDouble(sec, "BogieYawInertia", value))
        bogie_yaw_inertia = value;
    if (cfg.getDouble(sec, "BodyYawInertia", value))
        body_yaw_inertia = value;

    if (cfg.getDouble(sec, "BogieOffset", value))
        bogie_offset_default = value;
    if (cfg.getDouble(sec, "WheelsetSpacing", value))
        wheelset_spacing = value;
    if (cfg.getDouble(sec, "Substep", value))
        substep = std::min(std::max(value, 0.0002), 0.005);

    if (cfg.getInt(sec, "NumBogies", ivalue))
        num_bogies = static_cast<std::size_t>(std::max(ivalue, 1));

    // Умолчание спинового коэффициента - по литературе: f33·a·γ
    if (!spin_set)
        spin_creep_coeff = creep_longitudinal * 0.0076 * conicity;

    // Геометрия: как в вертикальной динамике
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

    axle_offset.assign(num_axis, 0.0);
    axle_bogie.assign(num_axis, 0);

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
    axles.assign(num_axis, AxleState());
    bogies.assign(num_bogies, BogieState());
    body = AxleState();

    wheel_yq.assign(num_axis * 2, 0.0);
    wheel_lateral_force.assign(num_axis * 2, 0.0);
    wheel_creep_ratio.assign(num_axis * 2, 0.0);
    wheel_angle_of_attack.assign(num_axis, 0.0);
    wheel_contact_state.assign(num_axis * 2,
                               static_cast<std::uint8_t>(ContactState::Rolling));
    axle_rel_y.assign(num_axis, 0.0);

    body_mass = std::max(full_mass -
                         static_cast<double>(num_axis) * wheelset_mass -
                         static_cast<double>(num_bogies) * bogie_mass,
                         0.2 * full_mass);

    resetMetrics();

    // Критическая скорость виляния - по росту амплитуды собственной модели
    critical_speed = estimateCriticalSpeed();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehicleLateralDynamics::isEnabled() const
{
    return enabled;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehicleLateralDynamics::isReady() const
{
    return ready && static_cast<bool>(lateral_offset);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleLateralDynamics::setLateralOffsetSource(LateralOffsetFn fn)
{
    lateral_offset = std::move(fn);
    ready = true;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleLateralDynamics::step(double dt,
                                  double velocity,
                                  std::int8_t dir,
                                  double train_coord,
                                  double curvature,
                                  double friction,
                                  const std::vector<double>& axle_load_factors,
                                  double cant_mm)
{
    if (!enabled || !isReady() || dt <= 0.0)
        return;

    const auto substeps = static_cast<std::size_t>(
                std::min(std::ceil(dt / substep), 1000.0));

    const double h = dt / static_cast<double>(substeps);

    // Эквивалентное боковое ускорение с учётом возвышения наружного
    // рельса (Б16, ТЗ "Поперечная динамика" п.14-15):
    //     a_eq = v^2/R - g*h_cant/(2*b),  b - половина колеи.
    // Возвышение наклоняет плоскость пути: составляющая тяжести
    // g*sin(θ) = g*cant/(2b) вычитается из центробежной. Остаток
    // (недовозвышение) разгружает внутреннее колесо через перенос
    // нагрузки и попадает в критерий Надаля через нагрузку колеса Q
    const double cant_m = cant_mm * 1.0e-3;
    const double a_cant = Physics::g * cant_m / (2.0 * half_gauge);
    const double a_eq = velocity * velocity * curvature - a_cant;

    cant_deficiency = a_eq;

    // Нагрузки осей от вертикальной динамики (для насыщения крипа и Y/Q)
    std::vector<double> load_factors = axle_load_factors;
    if (load_factors.size() != num_axis)
        load_factors.assign(num_axis, 1.0);

    for (std::size_t i = 0; i < substeps; ++i)
    {
        integrateSubstep(h, velocity, dir, train_coord, curvature,
                         friction, cant_mm);
    }

    // Датчики Y/Q после шага (используются последние силы)
    const double beta = flange_angle * Physics::PI / 180.0;
    const double mu = std::max(friction, 0.05);
    const double nadal_limit = (std::tan(beta) - mu) / (1.0 + mu * std::tan(beta));

    derailment_criterion = false;

    const double static_wheel_load = full_mass * Physics::g /
            (2.0 * static_cast<double>(num_axis));

    for (std::size_t i = 0; i < num_axis; ++i)
    {
        // Угол набегания (ТЗ "Динамика тележек", п.5-7): рыскание колёсной
        // пары минус направление касательной пути под осью (в кривой
        // касательная поворачивается на curvature * смещение оси)
        const double path_disp = static_cast<double>(dir) * axle_offset[i];
        wheel_angle_of_attack[i] = axles[i].psi - curvature * path_disp;

        // Насыщение крипа: отклонение боковой скорости от безкриповой
        const double noslip_dy = velocity * axles[i].psi;
        const double creep_dev = std::abs(axles[i].dy - noslip_dy);
        const double creep_ratio = creep_dev /
                std::max(std::abs(velocity) * 0.01, 0.01);

        // Перенос нагрузки от поперечной составляющей в кривой (Б16):
        // используется эквивалентное ускорение a_eq = v^2/R - g*cant/(2b)
        // вместо чистой центробежной - возвышение компенсирует часть
        // переноса. Доля на колесо = ±a_eq*h_cm/(half_gauge*g)
        // (h_cm - высота ЦМ), знак - по стороне кривой.
        // Доля ограничена: нагрузка колеса не уходит в минус,
        // но полная разгрузка возможна.
        // Боковой ветер добавляет перенос той же формулой через
        // эквивалентное ускорение F/m на высоте приложения (ТЗ "43-47",
        // п.2: ветер -> крен -> разгрузка колёс -> риск схода)
        const double a_wind = wind_lateral_force /
                std::max(full_mass, 1000.0);

        const double transfer = std::clamp(
                    (a_eq * mass_center_height -
                     a_wind * wind_force_height) / (half_gauge * Physics::g),
                    -1.0, 1.0);

        for (int s = 0; s < 2; ++s)
        {
            const std::size_t idx = i * 2 + static_cast<std::size_t>(s);

            wheel_creep_ratio[idx] = creep_ratio;

            const double q = static_wheel_load * load_factors[i] *
                    (1.0 + (s == 0 ? -transfer : transfer));

            // Классификация состояния контакта (ТЗ, п.12)
            std::uint8_t state = static_cast<std::uint8_t>(ContactState::Rolling);

            if (std::abs(axle_rel_y[i]) > flange_clearance)
            {
                state = static_cast<std::uint8_t>(ContactState::FlangeContact);
            }
            else if (q < 100.0)
            {
                state = static_cast<std::uint8_t>(ContactState::LossOfContact);
            }
            else if (creep_ratio > 0.98)
            {
                state = static_cast<std::uint8_t>(ContactState::Sliding);
            }
            else if (creep_ratio > 0.15)
            {
                state = static_cast<std::uint8_t>(ContactState::PartialSlip);
            }
            else if (creep_ratio > 0.005)
            {
                state = static_cast<std::uint8_t>(ContactState::Adhesion);
            }

            wheel_contact_state[idx] = state;

            if (q > 100.0)
            {
                wheel_yq[idx] = std::abs(wheel_lateral_force[idx]) / q;

                if (wheel_yq[idx] > nadal_limit)
                    derailment_criterion = true;
            }
            else
            {
                // Колесо разгружено: контакт не держит - критерий нарушен
                wheel_yq[idx] = 10.0;
                derailment_criterion = true;
            }
        }
    }

    // RMS боковых колебаний кузова (прореживание 10 мс)
    metrics_timer += dt;
    if (metrics_timer >= 0.01)
    {
        metrics_timer = 0.0;

        lateral_window.push_back(body.y);
        lateral_sq_sum += body.y * body.y;

        while (lateral_window.size() > 500)
        {
            lateral_sq_sum -= lateral_window.front() * lateral_window.front();
            lateral_window.erase(lateral_window.begin());
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleLateralDynamics::integrateSubstep(double h,
                                              double velocity,
                                              std::int8_t dir,
                                              double train_coord,
                                              double curvature,
                                              double friction,
                                              double cant_mm)
{
    // Знаковая скорость (velocity уже со знаком dir): при реверсе знак
    // меняет кинематику Клингеля и крип-цели, поэтому его сохраняем.
    // В знаменателях релаксации - модуль, ограниченный снизу
    const double v_abs = std::max(std::abs(velocity), 0.1);
    const double v = std::copysign(v_abs, velocity);
    const double g = Physics::g;

    const double center_path = static_cast<double>(dir) * train_coord;
    const double travel = velocity * h;

    const double static_axle_load = full_mass * g / static_cast<double>(num_axis);
    const double static_wheel_load = 0.5 * static_axle_load;

    // Коэффициент сцепления в контакте (погода/песок - через friction)
    const double mu = std::max(friction, 0.05);

    // Квазистатическое поперечное ускорение в кривой с учётом возвышения
    // наружного рельса (Б16): центробежная v^2/R частично гасится
    // составляющей тяжести на наклонённом пути g*cant/(2b)
    // (a_eq = v^2/R - g*cant/(2b), знак - вдоль оси +y модели)
    const double a_cant = g * cant_mm * 1.0e-3 / (2.0 * half_gauge);
    const double a_curve = -(velocity * velocity * curvature) + a_cant;

    //--- Колёсные пары ---

    for (std::size_t i = 0; i < num_axis; ++i)
    {
        AxleState& axle = axles[i];
        const BogieState& bogie = bogies[axle_bogie[i]];

        // Боковое смещение оси пути под колёсной парой
        const double p = center_path +
                static_cast<double>(dir) * axle_offset[i] + travel;
        const double y_track = lateral_offset(p);

        // Положение относительно оси пути
        const double y_rel = axle.y - y_track;
        const double dy_rel = axle.dy;

        axle_rel_y[i] = y_rel;

        //--- Крип: релаксация к скорости без проскальзывания ---
        // Боковой крип обнуляется при dy = v*psi
        const double noslip_dy = v * axle.psi;
        const double mu_q = mu * static_wheel_load;

        // Эффективная жёсткость крипа с насыщением трением:
        // при большом крипе сила не превышает mu*Q
        const double f22_eff = std::min(creep_lateral,
                                        2.0 * mu_q * v_abs / 0.01);

        const double tau_y = std::max(wheelset_mass * v_abs / f22_eff, h);
        const double relax_y = std::exp(-h / tau_y);

        const double dy_before = axle.dy;

        axle.dy = noslip_dy + (axle.dy - noslip_dy) * relax_y;

        // Фактическая сила крипа (для остальных сил и датчиков)
        const double creep_force_y = -wheelset_mass *
                (dy_before - noslip_dy) * (1.0 - relax_y) / h;

        //--- Продольный крип: кинематика Клингеля ---
        // Нулевой продольный крип: psi_dot = -conicity*v*y/(half_gauge*r0)
        const double f11_eff = std::min(creep_longitudinal,
                                        2.0 * mu_q * half_gauge * v_abs / 0.01);

        const double tau_psi = std::max(
                    wheelset_yaw_inertia * v_abs / (2.0 * f11_eff * half_gauge * half_gauge),
                    h);
        const double relax_psi = std::exp(-h / tau_psi);

        // Радиальная установка тележки в кривой (Б15, ТЗ "Тележки"
        // п.3/7): наружное колесо идёт по большему радиусу - разность
        // путей колёс за время dt: dS = curvature*v*b_wheel*dt.
        // При общем радиусе качения r0 это разность угловых скоростей
        // колёс dw = curvature*v*b_wheel/r0; продольный крип, гася её,
        // доворачивает ось к радиальному положению со скоростью
        //     noslip_dpsi += -curvature*v*(b_wheel/r0)/2
        // (делится между колёсами пары). Благодаря этому угол набегания
        // в кривой - физический результат модели, а не допущение
        double noslip_dpsi = -conicity * v * y_rel /
                (half_gauge * wheel_radius);

        noslip_dpsi += -curvature * v * half_gauge /
                (2.0 * wheel_radius);

        axle.dpsi = noslip_dpsi + (axle.dpsi - noslip_dpsi) * relax_psi;

        //--- Гребень: односторонний упор за пределами люфта ---
        double flange_force = 0.0;

        const double over = std::abs(y_rel) - flange_clearance;

        if (over > 0.0)
        {
            const double side = (y_rel > 0.0) ? -1.0 : 1.0;

            flange_force = side * (flange_stiffness * over +
                                   flange_damping * dy_rel *
                                   ((side * dy_rel) < 0.0 ? 1.0 : 0.0));

            // Ограничение силы гребня
            flange_force = std::clamp(flange_force, -400.0e3, 400.0e3);
        }

        //--- Связь I ступени с тележкой (поперечная + рыскание со люфтом) ---
        const double anchor_y = bogie.y;
        const double anchor_dy = bogie.dy;

        const double spring_y = -primary_lateral_stiffness * (axle.y - anchor_y) -
                                primary_lateral_damping * (axle.dy - anchor_dy);

        // Люфт рыскания: внутри зоны усилие не растёт (ТЗ, п.17)
        const double yaw_rel = axle.psi - bogie.psi;
        const double yaw_eff = std::abs(yaw_rel) > primary_yaw_free_play
                ? yaw_rel - std::copysign(primary_yaw_free_play, yaw_rel)
                : 0.0;

        const double spring_psi = -primary_yaw_stiffness * yaw_eff;

        //--- Спиновый крип (ТЗ "Поперечная динамика", п.20) ---
        // В кривой контактное пятно вращается в плоскости контакта
        // (спин, w_spin = v/R): момент спинового крипа
        //     M_spin = c_spin * (v/R),  c_spin ~ f33*a*gamma
        // действует на рыскание колёсной пары (по направлению кривой)
        const double spin_torque = spin_creep_coeff * velocity * curvature;

        //--- Интегрирование колёсной пары ---
        // Крип уже учтён релаксацией скоростей: явно интегрируем только
        // негриповые силы (гребень, подвешивание, кривизна, спин)
        const double total_f = flange_force + spring_y +
                wheelset_mass * a_curve;

        axle.dy += h * total_f / wheelset_mass;
        axle.y += axle.dy * h;

        axle.dpsi += h * (spring_psi + spin_torque) /
                wheelset_yaw_inertia;
        axle.psi += axle.dpsi * h;

        // Датчики сил на колёса: крип и гребень распределяются по колёсам
        const double flange_side = (y_rel > flange_clearance) ? 1 :
                ((y_rel < -flange_clearance) ? 0 : -1);

        for (int s = 0; s < 2; ++s)
        {
            const std::size_t idx = i * 2 + static_cast<std::size_t>(s);

            double y_force = 0.5 * creep_force_y;

            if (flange_side == s)
                y_force += flange_force;

            wheel_lateral_force[idx] = y_force;
        }
    }

    //--- Тележки ---

    for (std::size_t k = 0; k < num_bogies; ++k)
    {
        BogieState& bogie = bogies[k];

        double force = 0.0;
        double torque = 0.0;

        // Реакции I ступени от своих колёсных пар (с учётом люфта)
        for (std::size_t i = 0; i < num_axis; ++i)
        {
            if (axle_bogie[i] != k)
                continue;

            const AxleState& axle = axles[i];

            force += primary_lateral_stiffness * (axle.y - bogie.y) +
                     primary_lateral_damping * (axle.dy - bogie.dy);

            const double yaw_rel = axle.psi - bogie.psi;
            const double yaw_eff = std::abs(yaw_rel) > primary_yaw_free_play
                    ? yaw_rel - std::copysign(primary_yaw_free_play, yaw_rel)
                    : 0.0;

            torque += primary_yaw_stiffness * yaw_eff;
        }

        // Возвратный момент тележки к равновесию относительно кузова
        // (ТЗ, п.16), с люфтом поворота (п.17)
        const double bogie_yaw_rel = bogie.psi - body.psi;
        const double bogie_yaw_eff =
                std::abs(bogie_yaw_rel) > bogie_yaw_free_play
                ? bogie_yaw_rel - std::copysign(bogie_yaw_free_play, bogie_yaw_rel)
                : 0.0;

        torque += -bogie_yaw_stiffness * bogie_yaw_eff;

        // II ступень и боковые упоры
        const double rel = bogie.y - body.y;
        const double over = std::abs(rel) - side_bearer_clearance;

        double secondary = -secondary_lateral_stiffness * rel -
                           secondary_lateral_damping * (bogie.dy - body.dy);

        if (over > 0.0)
        {
            secondary -= side_bearer_stiffness * over *
                    ((rel > 0.0) ? 1.0 : -1.0);
        }

        force += secondary + bogie_mass * a_curve;

        bogie.dy += h * force / bogie_mass;
        bogie.y += bogie.dy * h;

        bogie.dpsi += h * torque / bogie_yaw_inertia;
        bogie.psi += bogie.dpsi * h;
    }

    //--- Кузов ---

    double body_force = 0.0;

    for (std::size_t k = 0; k < num_bogies; ++k)
    {
        const BogieState& bogie = bogies[k];

        const double rel = bogie.y - body.y;
        const double over = std::abs(rel) - side_bearer_clearance;

        double f = secondary_lateral_stiffness * rel +
                   secondary_lateral_damping * (bogie.dy - body.dy);

        if (over > 0.0)
        {
            f += side_bearer_stiffness * over * ((rel > 0.0) ? 1.0 : -1.0);
        }

        body_force += f;
    }

    // Кузов: центробежная составляющая + боковая ветровая нагрузка
    // (ветер давит на боковую поверхность кузова, ТЗ "43-47", п.2)
    body_force += body_mass * a_curve + wind_lateral_force;

    const double accel = body_force / body_mass;

    body.dy += h * accel;
    body.y += body.dy * h;

    // Рыскание кузова: моменты от тележек через II ступень
    double body_torque = 0.0;

    for (std::size_t k = 0; k < num_bogies; ++k)
    {
        body_torque += -secondary_lateral_stiffness *
                bogie_offset[k] * (bogies[k].y - body.y - bogie_offset[k] * body.psi);
    }

    body.dpsi += h * body_torque / body_yaw_inertia;
    body.psi += body.dpsi * h;

    // Фильтрованное боковое ускорение кузова (~20 Гц)
    const double alpha = std::min(1.0, h / 0.008);
    body_lateral_accel += alpha * (accel - body_lateral_accel);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleLateralDynamics::estimateCriticalSpeed() const
{
    // Численная оценка: интегрируем изолированную колёсную пару с рысканием
    // относительно тележки (якорь фиксирован) при разных скоростях и ищем
    // границу роста амплитуды. Модель та же, что в integrateSubstep,
    // но без связей с тележкой (ниже критической скорости системы
    // устойчивость определяется именно колёсной парой)
    const double h = 0.002;
    const double t_sim = 8.0;
    const auto steps = static_cast<std::size_t>(t_sim / h);

    double v_stable = 0.0;
    double v_unstable = 0.0;

    for (double v_test = 5.0; v_test <= 90.0; v_test += 5.0)
    {
        // Начальное возмущение 5 мм
        double y = 0.005;
        double dy = 0.0;
        double psi = 0.0;
        double dpsi = 0.0;

        const double mu = 0.25;
        const double static_wheel_load = full_mass * Physics::g /
                (2.0 * static_cast<double>(num_axis));
        const double mu_q = mu * static_wheel_load;

        const double f22_eff = std::min(creep_lateral, 2.0 * mu_q * v_test / 0.01);
        const double tau_y = std::max(wheelset_mass * v_test / f22_eff, h);
        const double relax_y = std::exp(-h / tau_y);

        const double f11_eff = std::min(creep_longitudinal,
                                        2.0 * mu_q * half_gauge * v_test / 0.01);
        const double tau_psi = std::max(
                    wheelset_yaw_inertia * v_test /
                    (2.0 * f11_eff * half_gauge * half_gauge), h);
        const double relax_psi = std::exp(-h / tau_psi);

        double amplitude_prev = std::abs(y);
        bool growing = false;
        double max_amplitude = std::abs(y);

        for (std::size_t n = 0; n < steps; ++n)
        {
            // Крип-релаксация (неявный крип)
            const double noslip_dy = v_test * psi;
            dy = noslip_dy + (dy - noslip_dy) * relax_y;

            const double noslip_dpsi = -conicity * v_test * y /
                    (half_gauge * wheel_radius);
            dpsi = noslip_dpsi + (dpsi - noslip_dpsi) * relax_psi;

            // Гребень
            const double over = std::abs(y) - flange_clearance;
            double flange = 0.0;
            if (over > 0.0)
            {
                const double side = (y > 0.0) ? -1.0 : 1.0;
                flange = side * (flange_stiffness * over +
                                 flange_damping * dy * ((side * dy) < 0.0 ? 1.0 : 0.0));
            }

            // Подвешивание к фиксированной тележке
            const double spring_y = -primary_lateral_stiffness * y -
                                    primary_lateral_damping * dy;
            const double spring_psi = -primary_yaw_stiffness * psi;

            // Крип учтён релаксацией: явно только негриповые силы
            dy += h * (flange + spring_y) / wheelset_mass;
            y += dy * h;
            dpsi += h * spring_psi / wheelset_yaw_inertia;
            psi += dpsi * h;

            max_amplitude = std::max(max_amplitude, std::abs(y));

            // Отсечка: слишком большой рост - неустойчивость очевидна
            if (std::abs(y) > 10.0 * flange_clearance)
            {
                growing = true;
                break;
            }
        }

        // Рост: максимум вышел за пределы начального возмущения значительно
        if (max_amplitude > 3.0 * amplitude_prev)
            growing = true;

        if (growing)
        {
            v_unstable = v_test;
            break;
        }

        v_stable = v_test;
    }

    if (v_unstable <= 0.0)
        return 100.0;   // Устойчиво во всём диапазоне

    if (v_stable <= 0.0)
        return 5.0;     // Неустойчиво сразу

    // Линейная интерполяция границы
    return 0.5 * (v_stable + v_unstable);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleLateralDynamics::getBodyLateralPosition() const
{
    return body.y;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleLateralDynamics::getBodyYaw() const
{
    return body.psi;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleLateralDynamics::getBodyLateralAcceleration() const
{
    return body_lateral_accel;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleLateralDynamics::getBogieLateralPosition(std::size_t bogie) const
{
    if (bogie >= bogies.size())
        return 0.0;

    return bogies[bogie].y;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleLateralDynamics::getBogieYaw(std::size_t bogie) const
{
    if (bogie >= bogies.size())
        return 0.0;

    return bogies[bogie].psi;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleLateralDynamics::getBogieRelativeYaw(std::size_t bogie) const
{
    if (bogie >= bogies.size())
        return 0.0;

    return bogies[bogie].psi - body.psi;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleLateralDynamics::getWheelsetAngleOfAttack(std::size_t axle) const
{
    if (axle >= wheel_angle_of_attack.size())
        return 0.0;

    return wheel_angle_of_attack[axle];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
VehicleLateralDynamics::ContactState
VehicleLateralDynamics::getWheelContactState(std::size_t axle, int side) const
{
    const std::size_t idx = axle * 2 +
            static_cast<std::size_t>(side >= 0 ? side : 0);

    if (idx >= wheel_contact_state.size())
        return ContactState::Rolling;

    return static_cast<ContactState>(wheel_contact_state[idx]);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleLateralDynamics::setConicity(double value)
{
    conicity = std::min(std::max(value, 0.0), 0.5);

    // Коничность меняет границу виляния: пересчитываем
    // критическую скорость (ТЗ, п.29)
    critical_speed = estimateCriticalSpeed();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleLateralDynamics::getConicity() const
{
    return conicity;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleLateralDynamics::setWindLateralForce(double force_n,
                                                 double height_m)
{
    wind_lateral_force = force_n;
    wind_force_height = std::max(height_m, 0.1);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleLateralDynamics::getWindLateralForce() const
{
    return wind_lateral_force;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleLateralDynamics::setMassCenterHeight(double value)
{
    mass_center_height = std::max(value, 0.1);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleLateralDynamics::getWheelsetLateralPosition(std::size_t axle) const
{
    if (axle >= axles.size())
        return 0.0;

    return axles[axle].y;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleLateralDynamics::getWheelLateralForce(std::size_t axle, int side) const
{
    if (axle * 2 + static_cast<std::size_t>(side >= 0 ? side : 0)
            >= wheel_lateral_force.size())
        return 0.0;

    return wheel_lateral_force[axle * 2 + static_cast<std::size_t>(side)];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleLateralDynamics::getWheelYQ(std::size_t axle, int side) const
{
    if (axle * 2 + static_cast<std::size_t>(side >= 0 ? side : 0)
            >= wheel_yq.size())
        return 0.0;

    return wheel_yq[axle * 2 + static_cast<std::size_t>(side)];
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool VehicleLateralDynamics::isDerailmentCriterionViolated() const
{
    return derailment_criterion;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleLateralDynamics::getCriticalSpeed() const
{
    return critical_speed;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleLateralDynamics::getLateralRMS() const
{
    if (lateral_window.empty())
        return 0.0;

    return std::sqrt(std::max(lateral_sq_sum, 0.0) /
                     static_cast<double>(lateral_window.size()));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double VehicleLateralDynamics::getCantDeficiency() const
{
    return cant_deficiency;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void VehicleLateralDynamics::resetMetrics()
{
    lateral_sq_sum = 0.0;
    lateral_window.clear();
    metrics_timer = 0.0;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString VehicleLateralDynamics::getDebugMsg() const
{
    QString msg = QString("Lateral: y=%1 m a=%2 m/s^2 yaw=%3 rad "
                          "Vcrit=%4 km/h RMS=%5 mm hD=%6 m/s^2")
            .arg(body.y, 0, 'f', 4)
            .arg(body_lateral_accel, 0, 'f', 2)
            .arg(body.psi, 0, 'f', 5)
            .arg(critical_speed * 3.6, 0, 'f', 0)
            .arg(getLateralRMS() * 1000.0, 0, 'f', 1)
            .arg(cant_deficiency, 0, 'f', 2);

    for (std::size_t i = 0; i < axles.size(); ++i)
    {
        msg += QString(" y%1=%2mm").arg(i).arg(axles[i].y * 1000.0, 0, 'f', 1);
    }

    return msg;
}
