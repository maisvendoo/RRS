#include    <autopilot.h>
#include    <QLibrary>

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
Autopilot *loadAutopilot(QString lib_path)
{
    Autopilot *autopilot = nullptr;

    QLibrary lib(lib_path);

    if (lib.load())
    {
        GetAutopilot getAutopilot = reinterpret_cast<GetAutopilot>(lib.resolve("getAutopilot"));

        if (getAutopilot)
        {
            autopilot = getAutopilot();
        }
    }

    return autopilot;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::step(double t, double dt)
{
    velocity_control(t, dt);

    vigilance_control(t, dt);

    accel_meter->setVelocity(feedback->v_cur);
    accel_meter->step(t, dt);

    rb_timer->step(t, dt);
    sand_timer->step(t, dt);    

    Device::step(t, dt);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
QString Autopilot::getDbgMsg()
{
    double v_p = calcPredictVelocity(feedback->v_cur, dist_target, accel_meter->value());


    QString msg = QString(" | АВТОВЕДЕНИЕ | Vтек.: %1 км/ч | Vокр.: %2 км/ч | Vзад.: %3 км/ч| Уск.: %4 м/с2| Зад. уск.: %5 м/с2 | Прогноз Vцел.: %6 км/ч")
        .arg(feedback->v_cur, 4, 'f', 1)
        .arg(feedback->v_tau, 4, 'f', 1)
        .arg(v_ref, 4, 'f', 1)
        .arg(accel_meter->value(), 6, 'f', 2)
        .arg(-a_brake, 6, 'f', 2)
        .arg(v_p, 4, 'f', 1)
                      .arg(target_station_dist, 10, 'f', 1);

    if (is_timetable_ready)
    {
        msg += QString(" | Цель: %1 | дист.: %2")
                   .arg(timetable.stations[target_station_idx].name)
                   .arg(target_station_dist, 7, 'f', 1);
    }

    return msg;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::vigilance_control(double t, double dt)
{
    if (feedback == nullptr)
    {
        return;
    }

    // Есть сигнал контроля бдительности
    if (feedback->is_vigilance_control)
    {
        if (!rb_timer->isStarted())
        {
            // Жмем РБ
            press_RB();
            // Запускаем таймер
            rb_timer->start();
        }
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::velocity_control(double t, double dt)
{
    if (feedback == nullptr)
        return;

    // Счисление пути
    calcTargetDistance();

    // Скорость, заданная по графику
    v_ref = calcTimetableVelocity(target_station_dist);

    // Выбираем минимум между текущим ограничением и конструкционной скоростью
    v_ref = min(calcCurrentSpeedLimit(t, dt), v_ref);

    // Рассчитываем скорость по тормозной кривой до следующего ограничения
    // (если оно больше, ну и пусть :))) )
    v_ref = min(v_ref, calcBrakeCurveSpeed(feedback->v_lim_next, feedback->limit_dist));

    // Расчитываем скорость по тормозной кривой до ближайшего сигнала
    v_ref = min(v_ref, calcAlsnSpeed(feedback->alsn_code, feedback->signal_dist, v_target));

    // Минимальная целевая скорость (для предсказания тормозного пути)
    v_target = min(feedback->v_lim_next, v_target);

    if (feedback->v_lim_next < feedback->v_lim)
        dist_target = min(feedback->v_lim_next, feedback->signal_dist);
    else
        dist_target = feedback->signal_dist;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::load_config(CfgReader &cfg)
{
    QString secName = "Device";

    cfg.getDouble(secName, "V_constr", v_constr);
    cfg.getDouble(secName, "BrakeAccel", a_brake_ref);
    cfg.getDouble(secName, "RefLength", ref_length);
    cfg.getDouble(secName, "RefMass", ref_mass);

    ref_mass = ref_mass * 1000.0;

    cfg.getDouble(secName, "LeadDistance_RY", lead_dist_RY);
    cfg.getDouble(secName, "LeadDistance_Y", lead_dist_Y);
    cfg.getDouble(secName, "SpeedLimit_RY", v_lim_RY);
    cfg.getDouble(secName, "SpeedDisableRelease", v_disable_release);
    cfg.getDouble(secName, "dVTractionOff", dV_traction_off);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Autopilot::calcCurrentSpeedLimit(double t, double dt)
{
    double v_lim = feedback->v_lim;

    if (feedback->v_lim < feedback->v_lim_next)
    {
        // Запоминаем текущее, более строгое ограничение
        prev_v_lim = feedback->v_lim;
        // ЗАпоминаем длину хвоста, который надо затянуть
        tail_len = train_length;
    }
    else
    {
        // Затянут весь хвост
        if (tail_len <= 0)
        {
            v_lim = feedback->v_lim;
        }
        else // хвост не затянут, едем по старому ограничению
        {
            v_lim = prev_v_lim;
            tail_len -= feedback->v_cur *dt / Physics::kmh;
        }
    }

    return v_lim;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Autopilot::calcBrakeCurveSpeed(double v_target, double dist)
{
    double vt = v_target / Physics::kmh;

    a_brake = a_brake_ref * ref_mass * train_length / train_mass / ref_length;

    return sqrt(vt * vt + pf(2 * a_brake * dist)) * Physics::kmh;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Autopilot::calcAlsnSpeed(ALSN alsn_code, double signal_dist, double &v_target)
{
    double v_lim = v_constr;

    is_disable_release = false;

    switch (alsn_code)
    {
    case RED_YELLOW:

        v_target = 0.0;

        v_lim = cut(calcBrakeCurveSpeed(v_target, signal_dist - lead_dist_RY), 0.0, v_lim_RY);

        // Запрещаем отпускать тормоза - остановка
        if (feedback->v_cur <= v_disable_release)
        {
            is_disable_release = true;            
        }

        // Если запрещен отпуск и мы остановились - запрещаем движение
        if ( (feedback->v_cur <= 1.0 || signal_dist <= lead_dist_RY) && is_disable_release)
        {
            is_motion_allowed = false;
        }

        break;

    case YELLOW:

        v_target = v_lim_RY;

        v_lim = calcBrakeCurveSpeed(v_target, signal_dist - lead_dist_Y);
        is_motion_allowed = true;

        break;

    case NO_CODE:

        // Тут, по идее, будет происходить торможение, а проверка бдительности
        // не даст сорвать ЭПК
        v_lim = 40.0; // ????

        is_motion_allowed = true;

        break;

    case GREEN:

        is_motion_allowed = true;

        break;
    }

    return v_lim;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Autopilot::calcPredictVelocity(double v_cur, double dist, double accel)
{
    double v0 = v_cur / Physics::kmh;

    double tmp = v0 * v0 + 2 * accel *dist;

    double v_p = 0.0;

    if (tmp < 0)
    {
        return v_p;
    }
    else
    {
        v_p = sqrt(tmp) * Physics::kmh;
    }

    return v_predict_filter.process(v_p);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::initTimeTable()
{
    // Гравик инициализирован - выход
    if (is_timetable_ready)
    {
        return;
    }

    // График пуст - выход
    if (timetable.stations.empty())
    {
        return;
    }

    // Определяем текущее положение ПЕ, где мы установлены
    emit sigGetVehicleTrajPosition(&curr_traj_name, &curr_traj_coord);

    if (curr_traj_name.isEmpty())
    {
        return;
    }

    prev_traj_name = curr_traj_name;
    prev_traj_coord = curr_traj_coord;

    // Индек целевой станции - в самый конец графика
    target_station_idx = timetable.stations.size() - 1;

    // Определяем направление, куда нам проверять маршрут
    bool is_route_exists = false;

    // Проверяем "туда"
    emit sigIsRouteExists(curr_traj_name,
                          timetable.stations[target_station_idx].target_traj,
                          target_dir,
                          &is_route_exists);

    // Нет маршрута
    if (!is_route_exists)
    {
        // Проверяем обратно
        target_dir = -1;

        emit sigIsRouteExists(curr_traj_name,
                              timetable.stations[target_station_idx].target_traj,
                              target_dir,
                              &is_route_exists);

        // Нет маршрута - из нашего текущего положения не достижима даже конечная станция
        if (!is_route_exists)
        {
            // график хрень
            target_station_idx = -1;
            return;
        }
    }

    // Ищем ближайшую станцию по ходу нашего следования, исходя из нашего текущего положения
    while (is_route_exists)
    {
        if (target_station_idx == 0)
        {
            // Дошли до первой станции
            break;
        }

        target_station_idx--;

        emit sigIsRouteExists(curr_traj_name,
                              timetable.stations[target_station_idx].target_traj,
                              target_dir,
                              &is_route_exists);

    }

    if (!is_route_exists)
    {
        target_station_idx++;
    }

    // Определяем дистанцию до ближайшей станции
    emit sigGetRouteLength(curr_traj_name, curr_traj_coord,
                           timetable.stations[target_station_idx].target_traj,
                           timetable.stations[target_station_idx].coord,
                           target_dir, &target_station_dist);

    is_timetable_ready = true;    
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::calcTargetDistance()
{
    if (!is_timetable_ready)
    {
        return;
    }

    //prev_traj_coord = curr_traj_coord;
    emit sigGetVehicleTrajPosition(&curr_traj_name, &curr_traj_coord);

    // Если сменилась текущаяя траектория - нечего делать, дергаем
    // топологию в поисках новой дистанции
    //if (curr_traj_name != prev_traj_name)
    //{
    emit sigGetRouteLength(curr_traj_name, curr_traj_coord,
                           timetable.stations[target_station_idx].target_traj,
                           timetable.stations[target_station_idx].coord,
                           target_dir, &target_station_dist);



        //prev_traj_name = curr_traj_name;
        //prev_traj_coord = curr_traj_coord;

    /*    return;
    }

    // Если мы на прежней траектории - совершенно незачем дергать топологию,
    // вычисляем новую дистанцию по смещению вдоль траектории
    target_station_dist -= qAbs(curr_traj_coord - prev_traj_coord);*/
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Autopilot::calcTimetableBrakeCurve(double dist)
{
    double v_ref = v_constr;

    return v_ref;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
double Autopilot::calcTimetableVelocity(double dist)
{
    double v_ref = v_constr;

    return min(v_ref, calcTimetableBrakeCurve(dist));
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::slotSetBrakeAccel(double a_brake)
{
    this->a_brake = a_brake;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::setTimetable(const autopilot_timetable_t &timetable)
{
    if (timetable.stations.empty())
    {
        return;
    }

    this->timetable = timetable;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::slotVigilanceControl()
{
    release_RB();
    rb_timer->stop();
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::slotSandTimer()
{
    sand_timer->stop();

    sand_OFF();
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void Autopilot::slotIncTargetStation()
{
    target_station_idx++;

    if (target_station_idx > timetable.stations.size() - 1)
    {
        target_station_idx = timetable.stations.size() - 1;
    }
}
